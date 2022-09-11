//This code is to control the hardware in the project(R522,leds,buzzer,electric lock ect..)
//via the GPIO pins
//the RC522 legs connection is: SDA=>D8,SCK=>D5,MOSI=>D7,MISO=>D6,IRQ=>none,RST=>D0
//all other hardware is connected as describe below

#include "memory.h"
#include <SPI.h>
#include <MFRC522.h>

//------------------------------------
//hardware define
#define gateRelay D2
#define speaker_pin D4
#define SS_PIN D8
#define RST_PIN D0
MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;
// Init array that will store new NUID
byte nuidPICC[4];



int gateOpenTime = 1000; //how much time the gate will be open(in millis)
int speakerTone = 4000;
int gateCloseTime = 200;
String content = "";//UID read content
void hardware_init(){
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  pinMode(gateRelay, OUTPUT);
  digitalWrite(gateRelay, LOW);
  tone(speaker_pin, speakerTone, gateCloseTime);
}


void makeSound(String operation){
  if(operation == "success"){
    tone(speaker_pin, speakerTone, gateOpenTime);
  }else if(operation == "failed"){
    for(int i=0;i<2;i++){
      tone(speaker_pin, speakerTone, gateCloseTime);
      delay(gateCloseTime*2);
    }
    delay(200);
    tone(speaker_pin, speakerTone, gateCloseTime*2);    
  }
  
}
//void openGate() {
//  Serial.println("Authorized access");
//  digitalWrite(green_led_pin, HIGH);
//  digitalWrite(red_led_pin, LOW);
//  tone(speaker_pin, 2400, gateOpenTime);
//  delay(gateOpenTime);
//  digitalWrite(green_led_pin, LOW);
//  digitalWrite(red_led_pin, HIGH);
//
//
//}

void openGate(){
  Serial.println("Authorized access");
  digitalWrite(gateRelay, HIGH);
  makeSound("success");
  delay(gateOpenTime);
  digitalWrite(gateRelay, LOW);
}

void AccessDenied() {
  Serial.println(" Access denied");
  makeSound("failed");
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
      AccessDenied();
    } 
    else{
      Serial.println(content+ " CARD DO EXIST");  
      openGate();
    }
  delay(800);
}


//TODO check if the card is already exist
String addNewTag() {
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



//habdle button press
//void waitFotButton() {
//  bool buttonState = false;
//  buttonState = digitalRead(button_pin);
//  if (buttonState == HIGH) {
//    delay(800);
//    Serial.println(" button pressed");
//  }
//}
