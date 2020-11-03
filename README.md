# Magicbit-Azure

This is a sample project for connecting your Magicbit to Azure. Magicbit is an ESP32 based development board for IoT. Therefore most of these libraries will work with your generic ESP32 board as well

## Prerequisites
* Setup the Magicbit board for Arduino by following the these instructions [Getting started with Magicbit](https://magicbit-arduino.readthedocs.io/en/latest/#installation-instructions)
* Install PubSubClient from Arduino libraries manager or from Git https://github.com/knolleary/pubsubclient
* Install ESPAsyncWebServer from Arduino libraries manager or from Git https://github.com/me-no-dev/ESPAsyncWebServer


Setting up
-------------------------------

- Clone this repository to your Arduino projects folder
- Open the Magicbit-Azure.ino file:

```c++
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
  ```

This is the bare minimum sketch that should be uploaded for the device to connect to the Azure
- This assumes that you have already created an Azure account and provisioned the IoT Hub. If not you can follow these instructions from official Azure docs https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-create-through-portal#create-an-iot-hub
- Upload to your Magicbit board. Wait for the board to reset
- With your PC or phone search for its configuration WiFi access point to come up. It would be named "MagicBit" in the default configuration
- Connect to this WiFi access point and go to http://192.168.4.1

- Fill in the following:
  1. Name: Name of your device's configuration WiFi access point. This is not relevant to IoT Hub
  2. Device Id : device Id of the created device in IoT Hub
  3. IoT Hub name: name of your Azure IoT Hub.
  4. Key: SAS token for the device. This can be generated using the generateSAS.js script in /web folder by running the following
```
node generateSAS.js <IoT Hub Host name> <Device ID> <Shared Access Key>
```
The shared access key can be found in the primary connection string for the device which is of the format HostName=<IoT Hub Name>.azure-devices.net;DeviceId=<Device Id>;SharedAccessKey=<shared access key>
  5. Select the WiFi access point and enter the password
- Save the configuration. The default admin password is 12345. You can change this from the **Admin** tab

- Once saved, the MagicBit will restart and try to connect to the WiFi and then to Azure. You can connect to the Magicbit's configuration WiFi AP and navigate to http://192.168.4.1 to check the status of the connection. 
- Login to Azure portal and try to send a Message to Device. It should come up in the Serial terminal.
