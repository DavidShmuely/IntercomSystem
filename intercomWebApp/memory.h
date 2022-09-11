
char deviceName[] = "Intercom System";

int uidCount = 0; //nuber of uid cards
const int maxUidTags = 10;
struct uidTag {
  String username;
  String color;
  String uid;
};
struct uidTag uidTags[maxUidTags];


bool is_card_exist(String uid) {
  bool is_equal = false;
  for (int i = 0 ; i < uidCount ; i++) {
    Serial.println("check  if "+uidTags[i].uid+"==  \""+uid+"\"");
    if("\""+uid+"\"" == uidTags[i].uid){
      is_equal = true;
      Serial.println("True");
      break;
    }
   }
    if (is_equal) {
      return true;
    }
  return false;
}



//void delete_card(String uid) {
//  // look for the uid location
//  int location = 0;
//  for (int j = 0 ; j < memory.n ; j++) {
//    bool is_equal = true;
//    for (int i = 0 ; i < UID_SIZE ; i++) {
//      if (memory.uids[j][i] != uid[i]) {
//        is_equal = false;
//        break;
//      }
//    }
//    if (is_equal) {
//      Serial.println("location = " + String(j));
//      for (int v = j ; v < memory.n ; v++) {
//        for (int k = 0 ; k < UID_SIZE ; k++) {
//          memory.uids[v][k] = memory.uids[v + 1][k];
//        }
//      }
//      memory.n--;
//      WRITE_MEMORY;
//      Serial.println("Card deleted successfully: "+uid);
//      break;
//    }
//  }
//}
