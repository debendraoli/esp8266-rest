/*
 * Author Debendra Oli
 * @github.com/debendraoli
 * twitter @debendraoli
 */

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

const char* ssid = "WIFI_SSID";
const char* password = "password";
const char* authToken = "RANDOM_AUTH_TOKEN";


bool output1State = false;
bool output2State = false;
bool output3State = false;
bool output4State = false;
bool output5State = false;
bool output6State = false;
bool output7State = false;
bool output8State = false;


const int output1 = 1;
const int output2 = 2;
const int output3 = 3;
const int output4 = 4;
const int output5 = 5;
const int output6 = 6;
const int output7 = 7;
const int output8 = 8;

ESP8266WebServer server(80);

void setCrossOrigin(){
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    server.sendHeader(F("Access-Control-Allow-Methods"), F("POST,GET,OPTIONS"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("Content-Type,Authorization"));
};

void sendCrossOriginHeader(){
    Serial.println(F("sendCORSHeader"));

    server.sendHeader(F("access-control-allow-credentials"), F("false"));

    setCrossOrigin();

    server.send(204);
}

bool toggle(char state) {
  if (state) {
    return HIGH;
  }
  return LOW;
}

void getSwitchStatus() {
  setCrossOrigin();
  if (server.hasHeader("Authorization")){
    if (authToken != server.header("Authorization")) {
      server.send(401, "application/json", "{\"message\": \"Auth error\"}");
      return;
    }
  }
  StaticJsonDocument<80> doc;
  doc["1"] = output1State;
  doc["2"] = output2State;
  doc["3"] = output3State;
  doc["4"] = output4State;
  doc["5"] = output5State;
  doc["6"] = output6State;
  doc["7"] = output7State;
  doc["8"] = output8State;
  String buf;
  serializeJson(doc, buf);
  server.send(200, "application/json", buf);
}



void turnSwitches() {
  setCrossOrigin();

  if (server.hasHeader("Authorization")){
    if (authToken != server.header("Authorization")) {
      server.send(401, "application/json", "{\"message\": \"Auth error\"}");
      return;
    }
  }

  if (server.hasArg("plain")== false){
    server.send(400, "application/json", "{\"message\": \"No body received\"}");
    return;
    }
  String postBody = server.arg("plain");
  StaticJsonDocument<80> doc;
  DeserializationError error = deserializeJson(doc, postBody);
  if (error) {
    Serial.print(F("Error parsing JSON "));
    Serial.println(error.c_str());
    String msg = error.c_str();
    Serial.println("parseObject() failed");
    server.send(400, "application/json", "{\"message\": \"Invalid body received\"}");
    return;
  }

    if (output1State != doc["1"]) {
      digitalWrite(output1State, toggle(doc["1"]));
      output1State = doc["1"];
      doc["1"] = output1State;
     }
    if (output2State != doc["2"]) {
      digitalWrite(output2State, toggle(doc["2"]));
      output2State = doc["2"];
      doc["2"] = output2State;
     }

     if (output3State != doc["3"]) {
      digitalWrite(output3State, toggle(doc["3"]));
      output3State = doc["3"];
      doc["3"] = output3State;
     }


     if (output4State != doc["4"]) {
      digitalWrite(output4State, toggle(doc["4"]));
      output4State = doc["4"];
      doc["4"] = output4State;
     }

     if (output5State != doc["5"]) {
      digitalWrite(output5State, toggle(doc["5"]));
      output5State = doc["5"];
      doc["5"] = output5State;
     }

     if (output6State != doc["6"]) {
      digitalWrite(output6State, toggle(doc["6"]));
      output6State = doc["6"];
      doc["6"] = output6State;
     }

     if (output7State != doc["7"]) {
      digitalWrite(output7State, toggle(doc["7"]));
      output7State = doc["7"];
      doc["7"] = output7State;
     }

     if (output8State != doc["8"]) {
      digitalWrite(output8State, toggle(doc["8"]));
      output8State = doc["8"];
      doc["8"] = output8State;
     }
    String buf;
    serializeJson(doc, buf);
    server.send(200, "application/json", buf);
}


void restServerRouting() {
    server.on(F("/"), HTTP_OPTIONS, sendCrossOriginHeader);
    server.on(F("/"), HTTP_GET, getSwitchStatus);
    server.on(F("/switch"), HTTP_OPTIONS, sendCrossOriginHeader);
    server.on(F("/switch"), HTTP_POST, turnSwitches);
}

void handleNotFound() {
  String message = "{\"status\": \"Not Found\", \"details\": {";
  message += "\"uri\": \"" + server.uri() + "\",";
  message += "\"method\": \"";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\",";
  message += "\"args\": {\"raw\":\"";
  message += server.args();
  message += "\",";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += "\"" + server.argName(i) + "\": \"" + server.arg(i) + "\"";
  }
  message += "}}";
  server.send(404, "application/json", message);
}

void setup(void) {
  pinMode(output1State, OUTPUT);
  pinMode(output2State, OUTPUT);
  pinMode(output3State, OUTPUT);
  pinMode(output4State, OUTPUT);
  pinMode(output5State, OUTPUT);
  pinMode(output6State, OUTPUT);
  pinMode(output7State, OUTPUT);
  pinMode(output8State, OUTPUT);
  digitalWrite(output1State, LOW);
  digitalWrite(output2State, LOW);
  digitalWrite(output3State, LOW);
  digitalWrite(output4State, LOW);
  digitalWrite(output5State, LOW);
  digitalWrite(output6State, LOW);
  digitalWrite(output7State, LOW);
  digitalWrite(output8State, LOW);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
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
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
