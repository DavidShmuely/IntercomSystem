#include "http_server.h"


String uidTagRead = "";
int incomingData  = 0;

void setup() {
  Serial.begin(115200);
  hardware_init();
  initWifi();
  serverBegin();
  Serial.println("1 - print memory");
  Serial.println("2 - add new card");
  Serial.println("3 - print EEPROM");
  Serial.println("4 - reset EEPROM");
}


void loop() {
  server.handleClient();
  waitFotButton();
  waitForUID(); 
  
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingData = Serial.parseInt();
  }
  if (incomingData == 1){
    Serial.println("you pressed 1");
    printUidMemory();
  }
  if (incomingData == 2){
    Serial.println("you pressed 2");
    String new_card = lookForNewCards();
    if(new_card != "NULL"){
      Serial.println("add_card(new_card):"+new_card);
      add_card(new_card);
    }   
  }
  if (incomingData == 3){
    Serial.println("you pressed 3");
    printMemory();
  }if (incomingData == 4){
    Serial.println("you pressed 4");
    memSet(8);
  }
    
}
