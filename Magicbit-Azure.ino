#include "MagicBit.h"
MagicBit device;
void setup(){
  device.init();
}
void loop(){
  device.loop();
}
bool onReceiveCustom(char* topic,char* payload,int length){
  
  return true;
}
