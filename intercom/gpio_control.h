//This code is to control the hardware in the project(R522,leds,buzzer,electric lock ect..)
//via the GPIO pins
//the RC522 legs connection is: SDA=>D8,SCK=>D5,MOSI=>D7,MISO=>D6,IRQ=>none,RST=>D0
//all other hardware is connected as describe below

#include "memory_tools.h"
#include <SPI.h>
#include <MFRC522.h>

//------------------------------------
//hardware define
#define green_led_pin D4
#define red_led_pin D3
#define speaker_pin D2
#define button_pin D1
#define SS_PIN D8
#define RST_PIN D0
MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;
// Init array that will store new NUID
byte nuidPICC[4];

int gateOpenTime = 1000; //how much time the gate will be open(in millis)
String content = "";//UID read content
void hardware_init(){
  EEPROM_INIT;
  UID_memory_init();
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  pinMode(button_pin, INPUT);
  pinMode(green_led_pin, OUTPUT);
  pinMode(red_led_pin, OUTPUT);  
  digitalWrite(green_led_pin, LOW);
  digitalWrite(red_led_pin, HIGH);
}

void waitForUID() {
  content = "";
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (i > 0) {
      content.concat(" ");
    }
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  content.toUpperCase();
  if(content == "NULL" || !is_card_exist(content)){
      Serial.println(content+ " CARD DOES NOT EXIST");
    } 
    else{
      Serial.println(content+ " CARD DO EXIST");  
    }
  delay(800);
}


//TODO check if the card is already exist
String lookForNewCards() {
  // Look for new cards
  Serial.println("Waiting to read new card");
  long int starttime = millis();
  long int endtime = starttime;
  while ((endtime - starttime) <= 3000) { // do this loop for up to 3000mS
    waitForUID();
    if(content!=""){
      return content;
    }
  }
  //when no new card readed
  Serial.println("no card added");

  return "NULL";
}


void OpenGate() {
  Serial.println("Authorized access");
  digitalWrite(green_led_pin, HIGH);
  digitalWrite(red_led_pin, LOW);
  tone(speaker_pin, 2400, gateOpenTime);
  delay(gateOpenTime);
  digitalWrite(green_led_pin, LOW);
  digitalWrite(red_led_pin, HIGH);


}
void AccessDenied() {
  Serial.println(" Access denied");
  digitalWrite(green_led_pin, LOW);
  for (int i = 0; i < 3; i++) {
    digitalWrite(red_led_pin, LOW);
    tone(speaker_pin, 1500, 400);
    delay(400);
    digitalWrite(red_led_pin, HIGH);
    delay(400);
  }

}

//habdle button press
void waitFotButton() {
  bool buttonState = false;
  buttonState = digitalRead(button_pin);
  if (buttonState == HIGH) {
    delay(800);
    Serial.println(" button pressed");
  }
}
