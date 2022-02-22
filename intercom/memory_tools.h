#include <EEPROM.h>

#define EEPROM_LENGTH 512
#define EEPROM_INIT EEPROM.begin(EEPROM_LENGTH);

//------------------------------------
//memory define
#define UID_SIZE  11 //in DEC
#define MAX_UID_COUNT 10 // number of cards
#define ADDRESS 0 //eeprom starting address

//lets define our struct

struct uid_memory {
  char uids[MAX_UID_COUNT][UID_SIZE];
  unsigned int n;//number of cards we have
  int special;
};

void memPut(int address, char* data, int len) {
  for (int i = 0; i < len; i++) {
    EEPROM.write(address + i, data[i]); //Write one by one with starting address of 0x0F
  }
  EEPROM.commit();    //Store data to EEPROM

}

String memGet(int address , int uidLength) {
  String cardID = "";
  for (int i = 0; i < uidLength; i++){
    cardID = cardID + char(EEPROM.read(address + i)); //Read one by one with starting address of "address"
  }
  return cardID;
}


//at init
static uid_memory memory;
#define WRITE_MEMORY memPut(ADDRESS, (char*)(&memory), sizeof(memory))

void UID_memory_init () {
  EEPROM.get(ADDRESS, memory);
  if (memory.special != 555) { //now we know this is the first time ever to use the memory stuct - we should init it
    memset(&memory, 0, sizeof(memory));
    memory.n = 0;
    memory.special = 555;
    WRITE_MEMORY;
  }
}

void add_card(String uid) {
  if (memory.n < MAX_UID_COUNT) {
    for (int i = 0 ; i < UID_SIZE ; i++) {
      memory.uids[memory.n][i] = uid[i];
    }
    memory.n += 1;//advance n
    WRITE_MEMORY;
  }
  else {
  Serial.println("add_card error -> memory full!");
  }
}

bool is_card_exist(String uid) {
  for (int j = 0 ; j < memory.n ; j++) {
    bool is_equal = true;
    for (int i = 0 ; i < UID_SIZE ; i++) {
      if (memory.uids[j][i] != uid[i]) {
        is_equal = false;
        break;
      }
    }
    if (is_equal) {
      return true;
    }
  }
  return false;
}

void printUidMemory() {
  for (int i = 0; i < memory.n; i++) {
    Serial.print("cardUID number:" + i);
    Serial.println(" " + memGet(((ADDRESS + UID_SIZE)*i), UID_SIZE));
    Serial.println();
  }
}

void printMemory(){
  Serial.println("printMemory begin");
  Serial.println("EEPROM_LENGTH = " + EEPROM_LENGTH);
  for (int i = 0 ; i < EEPROM_LENGTH ; i++) {
    byte value = EEPROM.read(i);
    Serial.print(i);
    Serial.print("\t");
    Serial.print(value, HEX);
    Serial.println();
  }
  Serial.println("printMemory end");
}

void memSet(byte value){
  for (int i = 0 ; i < EEPROM_LENGTH ; i++) {
    EEPROM.write(i, value);
  }
  EEPROM.commit();
}
