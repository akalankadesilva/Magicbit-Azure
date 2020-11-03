#pragma once
#include "Drivers.h"
#include "Config.h"
#include "Globals.h"
#include "PubSubClient.h"
#include <WiFiClientSecure.h>

#define WIFI_TIMEOUT 30000
#define MB_TIMEOUT 30000
#define MIN_TX_INTERVAL 500

bool isMqttConnected();
bool isWiFiConnected();
extern bool onReceiveCustom(char* topic,char* payload,int length);
class MagicBit{

public:
	PubSubClient *mqtt;
	WiFiClientSecure modem;
	void init();
	void loop();
	bool sendPayload(char* payload,int length);

private:
	char buffer[256];
	int readLength = 0;
	unsigned long lastWiFiCheck = 0;
	unsigned long lastTx = 0;
	bool isSetup = false;
	char tick[2]={0xFA,0x00};
	
	bool wifiInit();
	bool mqttInit();
};