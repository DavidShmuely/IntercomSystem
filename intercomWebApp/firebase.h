#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
//#include <WiFiClient.h>
#include "hardware_control.h"

/* 1. Define the WiFi credentials */
//#define WIFI_SSID "Shmuely2.4"
//#define WIFI_PASSWORD "0547615838"

#define WIFI_SSID "shiran"
#define WIFI_PASSWORD "0542436438"

/* 2. If work with RTDB, define the RTDB URL and database secret */
#define DATABASE_URL "intercom-21cfe-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "BwAtJwlUFS2q9zgZGJMSDNHTDbn8xSo9T7fw1JYM"

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

//  wifi configuration
//----------------------------------------------------------------------
void initWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

}

//   Firebase Realtime Database Configuration.
//----------------------------------------------------------------------
void firebaseInit() {
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;

  Firebase.reconnectWiFi(true);

  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);

  // In setup(), set the streaming path to "/test/data" and begin stream connection
  if (!Firebase.RTDB.beginStream(&fbdo, "/request/name")){
    Serial.println(fbdo.errorReason());
  }



  //  Firebase.setString("ipAddress/shitritIntercom",WiFi.localIP().toString()); //--> update the Firebase Realtime Database with intercom IP address
}


//----------------------------------------------------------------------

void getTagsFromDatabase() {
  //get the numbers of uid in the DB
  if (Firebase.RTDB.getInt(&fbdo, "/uidTags/uidCount")) {
    if (fbdo.dataType() == "int") {
      uidCount = fbdo.to<int>() ;
      Serial.println("uidCount: " + String(uidCount));
    }
  } else {
    Serial.println(fbdo.errorReason());
  }

  //get the firebase data
  if (Firebase.RTDB.get(&fbdo, "/uidTags")){
    // Success, then try to read the JSON payload value
    FirebaseJson &json = fbdo.to<FirebaseJson>();
//    Serial.println("\n---------");
//    json.toString(Serial, true); // serialize contents to serial
    
    FirebaseJsonData result;
    for (int i = 0; i < uidCount; i++) {
      json.get(result, "tag" + String(i));//To get the json object from deserializing result
      if (result.success){
        if (result.type == "object") /* or result.typeNum == FirebaseJson::JSON_OBJECT */
        {
          FirebaseJson json2;
          result.get<FirebaseJson /* type e.g. FirebaseJson or FirebaseJsonArray */>(json2 /* object that used to store value */);
//          Serial.println("\n---------");
//          json2.toString(Serial, true); // serialize contents to serial

          // To iterate all values in Json object
          size_t count = json2.iteratorBegin();
          Serial.println("\n---------");
          for (size_t j = 0; j < count; j++){
            FirebaseJson::IteratorValue value = json2.valueAt(j);
            Serial.printf("Name: %s, Value: %s, Type: %s\n", value.key.c_str(), value.value.c_str(), value.type == FirebaseJson::JSON_OBJECT ? "object" : "array");
            if(value.key == "name"){
              uidTags[i].username = value.value;
            }
            if(value.key == "color"){
              uidTags[i].color = value.value;
            }
            if(value.key == "uid"){
              uidTags[i].uid = value.value;
            }
          }
          Serial.println();
          json2.iteratorEnd(); // required for free the used memory in iteration (node data collection)
        }
      }
    }
  }
  else
  {
    // Failed to get JSON data at defined node, print out the error reason
    Serial.println(fbdo.errorReason());
  }


}

//TODO: check what to do if the stream timeout
void eventListener(){
  if (!Firebase.RTDB.readStream(&fbdo)){
    Serial.println(fbdo.errorReason());
  }
  
  if (fbdo.streamTimeout()){
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
  
  if (fbdo.streamAvailable()){
      Serial.println("Stream at /request/name: "+fbdo.to<String>());
      if(fbdo.to<String>() == "openGate"){
        openGate();
        Firebase.RTDB.set(&fbdo,"/request/status", "success") ;
      }else if(fbdo.to<String>() == "refreshMemory"){
        getTagsFromDatabase();
        Firebase.RTDB.set(&fbdo,"/request/status", "success") ;
      }else if(fbdo.to<String>() == "addNewTag"){
        String temp = addNewTag();
        if(temp == "NULL"){
          Firebase.RTDB.set(&fbdo,"/request/status", "failed") ;
        }else
          Firebase.RTDB.set(&fbdo,"/request/status", "success") ;
          //update the database with the new uidTag
          Firebase.RTDB.set(&fbdo,"/uidTags/tag"+String(uidCount)+"/uid", temp) ;
          Firebase.RTDB.set(&fbdo,"/uidTags/tag"+String(uidCount)+"/name", "--") ;
          Firebase.RTDB.set(&fbdo,"/uidTags/tag"+String(uidCount)+"/color", "transparent") ;
          uidCount++;
          Firebase.RTDB.set(&fbdo,"/uidTags/uidCount", uidCount) ;
        
      }
  }
}
