/*
 * github.com/debendraoli
 * twitter @debendraoli
*/

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

const char* ssid = "WIFI_SSID";
const char* password = "password";
const String authToken = "RANDOM_AUTH_TOKEN";

StaticJsonDocument<80> doc;

ESP8266WebServer server(80);

void saveStates() {
  File f=LittleFS.open("state","w"); //open as a brand new file, discard old contents
  if(f){
    String buf;
    serializeJson(doc, buf);
    f.println(buf);
    f.close();
  }
};

void loadState() {
  File f;
  f=LittleFS.open("state","r");
  if(f){
    String mod=f.readString();
    DeserializationError error = deserializeJson(doc, mod);
    if (error) {
        Serial.print(F("Error parsing JSON "));
        Serial.println(error.c_str());
        String msg = error.c_str();
        Serial.println("parseObject() failed");
        server.send(400);
        return;
    }
    f.close();
  } else {
      for (int i = 1; i != 8; i++) {
          doc[i] = false;
      }
    saveStates();
  }
};

void setCrossOrigin() {
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    server.sendHeader(F("Access-Control-Allow-Methods"), F("GET,OPTIONS"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("Authorization"));
    server.sendHeader(F("access-control-allow-credentials"), F("true"));
};

void sendCrossOriginHeader(){
    setCrossOrigin();
    server.send(204);
};

bool toggle(char state) {
  if (state) {
    return HIGH;
  }
  return LOW;
}

void switches() {
  setCrossOrigin();
  if (!server.hasHeader("Authorization")) {
    server.send(401);
    return;
  }
  if (authToken != server.header("Authorization")) {
    server.send(401);
    return;
  }

  String action = server.arg("action");

  if (action == "status") {
    String buf;
    serializeJson(doc, buf);
    server.send(200, "application/json", buf);
  } else if ( action == "toggle") {
    int pin = server.arg("switch").toInt();
    if (!pin > 0 || pin > 8 ) {
        server.send(400, "application/json", "{\"message\": \"Invalid pin range. allowed: 1-8\"}");
    }
    bool level = !doc[pin];
    digitalWrite(pin, toggle(level));
    doc[pin] = level;
    saveStates();
    server.send(200);
  } else {
    server.send(400, "application/json", "{\"message\": \"Action not supported. allowed: status, toggle\"}");
  }
  return;
}


void restServerRouting() {
    server.on(F("/"), HTTP_OPTIONS, sendCrossOriginHeader);
    server.on(F("/"), HTTP_GET, switches);
}

void handleNotFound() {
  server.send(404);
}


void setup(void) {
    loadState();
    for (int i = 1; i == 8; i++) {
        pinMode(i, OUTPUT);
        digitalWrite(i, toggle(doc[i]));
    }
    Serial.begin(115200);
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
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);

  const char * headerkeys[] = {"Authorization"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  // ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
