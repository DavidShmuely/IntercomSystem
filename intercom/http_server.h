#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "gpio_control.h"


#define STASSID "Shmuely2.4"
#define STAPSK  "0547615838"


const char* ssid = STASSID;
const char* password = STAPSK;
ESP8266WebServer server(80);
char deviceName[] = "Intercom System";
bool gate_state = LOW;//HIGH for open LOW for close



//----------------------------------------------------------------------
//HTML page handle

void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!\r\n");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void initWifi(){
  IPAddress local_IP(192, 168, 1, 108);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(local_IP, gateway, subnet);
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

  
}





void serverBegin(){
  //Handle HTTP
  //------------------------------------------------------------------
  server.on("/", handleRoot);
  server.on("/command", HTTP_PUT, []() {
    DynamicJsonDocument root(1024);
    DeserializationError error = deserializeJson(root, server.arg("plain"));
    if (error) {
      server.send(404, "text/plain", "FAIL. " + server.arg("plain"));
    } else {
      if (root.containsKey("command")) {
        if (root["command"] == "open_gate") {
          OpenGate();
        }
      }
      if (root.containsKey("command")) {
        if (root["command"] == "add_card") {
          //addNewCard();
        }
      }
      String output;
      serializeJson(root, output);
      server.send(200, "text/plain", output);
      //print the messege
      Serial.println("http put" + output);
      delay(200);
    }
  });

  server.on("/command", HTTP_GET, []() {
    DynamicJsonDocument root(1024);
    root["command"] = gate_state;
    String output;
    serializeJson(root, output);
    server.send(200, "text/plain", output);
    //print the messege
    Serial.println("http grt" + output);
  });

  //--------------------------------------------
  server.onNotFound(handleNotFound);
  

  //--------------------------------------------
  //Handle Reset HTTP command
  server.on("/reset", []() {
    server.send(200, "text/html", "reset");
    delay(100);
    ESP.restart();
  });
  //-------------------------------------------
  server.begin();
  Serial.println("HTTP server started");
}
