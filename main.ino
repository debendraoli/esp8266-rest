/*
 * github.com/debendraoli
 * twitter @debendraoli
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

const char* ssid = "SSID_NAME";
const char* password = "PASSWORD";
const String authToken = "Bearer RANDOM_AUTH_TOKEN";
const char* CONTENT_TYPE = "application/json";
const char* pin_mapping = "{\"1\":5,\"2\":4,\"3\":0,\"4\":2,\"5\":14,\"6\":12,\"7\":13,\"8\":15}";

ESP8266WebServer server(80);

void handleRoot();
void handleNotFound();
String save_state(StaticJsonDocument<150>);
StaticJsonDocument<150> load_state();
String gen_res_message(String);
void routes();


String save_state(StaticJsonDocument<150> doc) {
  File file = LittleFS.open("state","w");
  String buf;
  if(!file) {
    server.send(500);
  }
  serializeJson(doc, buf);
  if (!file.print(buf)) {
    Serial.println("Failed to open file for reading");
    server.send(500);
  }
  file.close();
  return buf;
}

StaticJsonDocument<150> load_state() {

  StaticJsonDocument<150> doc;
  File file = LittleFS.open("state","r");
  if (file) {
    while (file.available()) {
      DeserializationError error = deserializeJson(doc, file.readString());
      if (error) {
        server.send(500, CONTENT_TYPE, gen_res_message("Failed to load state"));
        return doc;
      }
    }
  } else {
    for (int i = 1; i != 9; i++) {
      String pin = String(i);
      doc[pin] = LOW;
    }
    String state = save_state(doc);
    DeserializationError error = deserializeJson(doc, state);
    if (error) {
      server.send(500, CONTENT_TYPE, gen_res_message("Failed to load state"));
      return doc;
    }
  }
  return doc;
}

String gen_res_message(String message) {
  StaticJsonDocument<100> doc;
  doc["message"] = message;
  String buf;
  serializeJson(doc, buf);
  return buf;
}

void handleRoot() {
  if (!server.hasHeader("Authorization") || authToken != server.header("Authorization")) {
    server.send(401, CONTENT_TYPE, gen_res_message("Authorization error."));
    return;
  }

  String action = server.arg("action");
  StaticJsonDocument<150> doc = load_state();
  String buf;

  if (action == "status") {
    serializeJson(doc, buf);
    server.send(200, CONTENT_TYPE, buf);
    return;
  } else if (action == "toggle") {
    String pin_str = server.arg("switch");
    int pin = pin_str.toInt();
    if (pin < 0 || pin > 8) {
        server.send(400, CONTENT_TYPE, gen_res_message("Invalid pin range. allowed: 1-8"));
        return;
    }
    Serial.print("Toggling:");
    Serial.println(pin);

    StaticJsonDocument<150> pin_maps;

    DeserializationError error = deserializeJson(pin_maps, pin_mapping);
    if (error) {
      server.send(500, CONTENT_TYPE, gen_res_message("Something went wrong."));
      return;
    }
    bool level = doc[pin_str];
    digitalWrite(pin_maps[pin_str], level);
    doc[pin_str] = !level;
    String state = save_state(doc);
    server.send(200, CONTENT_TYPE, state);
    return;
  } else {
    server.send(400, CONTENT_TYPE, gen_res_message("Action not supported. allowed: status and toggle."));
    return;
  }
}

void routes() {
    server.on("/", HTTP_GET, handleRoot);
    server.onNotFound(handleNotFound);
}

void handleNotFound() {
  server.send(404, CONTENT_TYPE, gen_res_message("No found."));
}


void setup(void) {

  Serial.begin(115200);
  while (!Serial) continue;

  while (!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    ESP.restart();
  }
  
  StaticJsonDocument<150> pin_maps;

  DeserializationError error = deserializeJson(pin_maps, pin_mapping);
  if (error) {
    return;
  }

  StaticJsonDocument<150> state = load_state();
  JsonObject documentRoot = state.as<JsonObject>();

  for (JsonPair keyValue : documentRoot) {
    String pin = keyValue.key().c_str();
    pinMode(pin_maps[pin], OUTPUT);
    digitalWrite(pin_maps[pin], !state[pin]);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(3000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  if (MDNS.begin("esp8266")) {        
    Serial.println("MDNS responder started");
  }

  // Set server routing
  routes();

  const char * headerkeys[] = {"Authorization"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  // ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
