//TODO: - upload file via HTTP
//      - setting variables
#include "firebase.h"



unsigned long lastMillis;

void setup() {
  Serial.begin(115200);
  hardware_init();
  initWifi();
  firebaseInit();
  getTagsFromDatabase();
}


void loop() {
  eventListener();
  //waitFotButton();
  waitForUID();

}
