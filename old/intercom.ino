#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


#define green_led_pin D4
#define red_led_pin D3
#define button_pin D0
#define speaker_pin D2

//----------------------------------------
//RC522 settings
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN D8
#define RST_PIN D0
MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;
// Init array that will store new NUID
byte nuidPICC[4];
struct rfidCardManager {
  String card1, card2, card3, card4, card5, card6, card7, card8, card9, card10;
  int cardCounter = 0;
};

//---------------------------------------



#ifndef STASSID
#define STASSID "Shmuely2.4"
#define STAPSK  "0547615838"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
ESP8266WebServer server(80);

bool gate_state;//HIGH for open LOW for close
char deviceName[] = "Intercom System";
int buttonState = 0; //button state - HIGH\LOW
int gateOpenTime = 1000; //how much time the gate will be open(in millis)

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


//---------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  Serial.println();
  pinMode(button_pin, INPUT);
  pinMode(green_led_pin, OUTPUT);
  pinMode(red_led_pin, OUTPUT);
  //chack the leds&buzzer on startup(the buzzer is connected to the green led)
  for (int i = 0; i < 3; i++) {
    digitalWrite(green_led_pin, HIGH);
    digitalWrite(red_led_pin, LOW);
    delay(200);
    digitalWrite(green_led_pin, LOW);
    digitalWrite(red_led_pin, HIGH);
    delay(200);
  }
  gate_state = LOW;

  //-------------------------------------------------------------------
  //WIFI Manager
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

  server.onNotFound(handleNotFound);

  //------------------------------------------------------------------
  //Handle HTTP
  server.on("/", handleRoot);
  server.on("/command", HTTP_PUT, []() {
    DynamicJsonDocument root(1024);
    DeserializationError error = deserializeJson(root, server.arg("plain"));
    if (error) {
      server.send(404, "text/plain", "FAIL. " + server.arg("plain"));
    } else {
      if (root.containsKey("command")) {
        if (root["command"] == "open_gate") {
          gate_state = HIGH;
          OpenGate();
        }
      }
      if (root.containsKey("command")) {
        if (root["command"] == "add_card") {
          addNewCard();
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
//-------------------------------------------
void OpenGate() {
  gate_state = HIGH;
  digitalWrite(green_led_pin, HIGH);
  digitalWrite(red_led_pin, LOW);
  tone(speaker_pin, 2400, gateOpenTime);
  delay(gateOpenTime);
  gate_state = LOW;
  digitalWrite(green_led_pin, LOW);
  digitalWrite(red_led_pin, HIGH);
}
void AccessDenied() {
  digitalWrite(green_led_pin, LOW);
  for (int i = 0; i < 3; i++) {
    digitalWrite(red_led_pin, LOW);
    tone(speaker_pin, 1500, 400);
    delay(400);
    digitalWrite(red_led_pin, HIGH);
    delay(400);
  }
  gate_state = LOW;
}
void addNewCard() {
  Serial.println("Waiting to read new card");
  digitalWrite(red_led_pin, LOW);
  digitalWrite(green_led_pin, HIGH);
  long int starttime = millis();
  long int endtime = starttime;
  while((endtime - starttime) <= 5000) { // do this loop for up to 5000mS
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      continue;
    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      continue;
    }
    //Show UID on serial monitor
    Serial.print("UID tag :");
    String content = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    content.toUpperCase();
    Serial.println("New card added "+content);
    tone(speaker_pin, 2400, 1000);
    delay(1000);
    digitalWrite(green_led_pin, LOW);
    digitalWrite(red_led_pin, HIGH);
    break;
  }
  //when no new card readed
  for (int i = 0; i < 3; i++) {
    digitalWrite(red_led_pin, LOW);
    tone(speaker_pin, 1500, 400);
    delay(400);
    digitalWrite(red_led_pin, HIGH);
    delay(400);
  }
}


void lookForNewCards() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
}

void loop() {
  server.handleClient();

  //habdle button press ->send messege to the app
  buttonState = digitalRead(button_pin);
  if (buttonState == HIGH) {
    delay(800);
    Serial.println(" button pressed");
  }

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if ((content.substring(1) == "B9 66 C2 55") /*|| (content.substring(1) == "F6 7F 0D F8")*/ ) //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    Serial.println();
    OpenGate();
  }
  else {
    Serial.println(" Access denied");
    AccessDenied();
  }
}
