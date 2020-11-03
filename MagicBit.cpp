#include "Magicbit.h"
static MagicBit * thisDevice;
static void callback(char* topic, byte* payload, unsigned int length) {
  payload[length]=0;
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.println(F("] "));
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if(!onReceiveCustom(topic,(char*)payload,length))
      onReceive((char*)payload,length);
}

bool isMqttConnected() {
  if(thisDevice==NULL)
    return false;
  if(thisDevice->mqtt==NULL)
    return false;
  return thisDevice->mqtt->connected();
}
bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}
void MagicBit::init() {
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(500);
  Serial.println("Starting..");
  WiFi.mode(WIFI_AP_STA);
  thisDevice=this;
  driverInit();
  delay(2000);
  if (checkErase()) {
    Serial.println("Factory Reset");
    displayTitle("Alert");
    displayText("Factory resetting..");
    eraseConfig();
    displayText("Factory reset complete!. Restarting..");
    delay(3000);
    ESP.restart();
  }
  clearDisplay();

  if (configInit()) {
    if (strlen(mConfig.name) > 0)
      WiFi.softAP(mConfig.name);
    else
      WiFi.softAP("MagicBit");
    displayTitle("Starting..");
    sprintf(buffer,"Connecting to %s...",mConfig.ssid);
    displayText(buffer);
    if (wifiInit()) {
      displayText("Connecting to Azure..");
      mqttInit();
    }
    else {
      displayTitle("Error");
      displayText("WiFi connection failed!");
    }
  }
  else {
    WiFi.softAP("MagicBit");
    displayTitle("Setup");
    sprintf(buffer,"Connect to \"MagicBit\" WiFi and go to http://192.168.4.1 in a browser to setup");
    displayText(buffer);
  }
}

bool MagicBit::wifiInit() {
  if (strlen(mConfig.ssid) > 0 && strlen(mConfig.wifiPassword) > 0) {
    unsigned long start = millis();
    Serial.print("Connecting to ");
    Serial.println(mConfig.ssid);
    WiFi.begin(mConfig.ssid, mConfig.wifiPassword);
    while (WiFi.status() != WL_CONNECTED && (millis() - start < WIFI_TIMEOUT)) {
      delay(1000);
      Serial.print(".");
    }
    return WiFi.status() == WL_CONNECTED;
  }
  return false;
}

bool MagicBit::mqttInit() {
  if (strlen(mConfig.deviceId) > 0 && strlen(mConfig.key) > 0) {
    isSetup = true;
    Serial.print("Connecting to Azure..");
    Serial.print("Device ID:");
    Serial.println(mConfig.deviceId);
    
    if(mqtt==NULL){
      char* hubName=(char*)malloc(128);
      sprintf(hubName,"%s.azure-devices.net",mConfig.hub);
      Serial.print("Hub:");
      Serial.println(hubName);
      mqtt=new PubSubClient(hubName, 8883, callback, (Client&)modem);
    }
    if(mqtt!=NULL && !mqtt->connected()){
      Serial.print("Username:");
      char* username=(char*)malloc(164);
      sprintf(username,"%s.azure-devices.net/%s/?api-version=2018-06-30",mConfig.hub,mConfig.deviceId);
      Serial.println(username);
      Serial.println("MQTT Connecting..");
      
      bool ret=mqtt->connect(mConfig.deviceId,username,mConfig.key);
      Serial.print("Connection Result:");
      Serial.println(ret);
      Serial.print(F("Subscribing D2C: "));
       char* tempBuff=(char*)malloc(256);
      sprintf(tempBuff,"devices/%s/messages/devicebound/#",mConfig.deviceId);
      ret=mqtt->subscribe(tempBuff);
      Serial.println(ret);
      Serial.print(F("Subscribing DT Res: "));
     memset(tempBuff,0,sizeof(tempBuff));
      strcpy(tempBuff,"$iothub/twin/res/#");
      ret=mqtt->subscribe(tempBuff);
      Serial.println(ret);
      Serial.print(F("Subscribing DT: "));
     memset(tempBuff,0,sizeof(tempBuff));
      strcpy(tempBuff,"$iothub/twin/PATCH/properties/desired/#");
      ret=mqtt->subscribe(tempBuff);
      Serial.println(ret);
      Serial.print("Requesting DT:");
      memset(tempBuff,0,sizeof(tempBuff));
      strcpy(tempBuff,"$iothub/twin/GET/?$rid=1");
      ret=mqtt->publish(tempBuff," ",1);
      Serial.println(ret);
      
      free(tempBuff);
    }

 
    unsigned long start = millis();
    
    if (!mqtt->connected()) {
      displayTitle("Error");
      Serial.println("Failed to connect to Azure");
      displayText("Failed to connect to Azure!");
    }
    else {
      Serial.println("Connected!");
      displayText("");
      displayTitle("");
      displayIcons(1);
      displayAlert("Connected!");
    }
    return true;
  }
  else {
    displayTitle("Setup");
    Serial.println("Azure id, key not found");
    sprintf(buffer,"Connect to \"%s\" WiFi and go to http://192.168.4.1 to setup deviceId & key",mConfig.name);
    displayText(buffer);
  }
  return false;
}

void MagicBit::loop() {
 if(mqtt!=NULL)
    mqtt->loop();
  driverLoop();
  if (readLength > 0) {
    onReceive(buffer, readLength);
  }
  if (txLength > 0 && (millis() - lastTx > MIN_TX_INTERVAL || millis() < lastTx)) {
   sendPayload(txBuffer, txLength);
    txLength = 0;
    lastTx = millis();
  }
  if (millis() - lastWiFiCheck > 10000 || millis() < lastWiFiCheck) {

    lastWiFiCheck = millis();
    if (!isSetup)
      return;
    if (WiFi.status() != WL_CONNECTED) {

      wifiInit();
    }
    if (mqtt->connected())
      displayIcons(1);
    else {
      displayIcons(2);
      if (WiFi.status() == WL_CONNECTED)
        if (mqttInit())
          displayIcons(1);
        else
           displayIcons(2);
    }
    redrawDisplay();
  }
}
bool MagicBit::sendPayload(char* payload,int length){
  if(mqtt==NULL){
    Serial.print(F("MQTT not initialized"));
    return false;
  }
  bool ret=false;
  char* tempBuff=(char*)malloc(256);
  sprintf(tempBuff,"devices/%s/messages/events/",mConfig.deviceId);
  ret=mqtt->publish(tempBuff,payload,length);
   Serial.print("Publish status: ");
  Serial.println(ret);
  free(tempBuff);
  return ret;
}