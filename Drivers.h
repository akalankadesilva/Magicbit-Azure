#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NewPing.h>
#include "ESP32Servo.h"
#include "DHT.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN  32
#define LED_COUNT 1


#define DHTPIN 32
#define DHTTYPE DHT11


#define OLED_RESET 4
#define ALERT_TIMEOUT 5000
#define PB_RIGHT 34
#define PB_LEFT 35
#define LDR 36
#define BUZZER 25
#define POT 39
#define LED_R 27
#define LED_Y 18
#define LED_G 16
#define LED_B 17
#define M1A 16
#define M1B 17
#define M2A 18
#define M2B 27
#define INT_PIN_COUNT 10
#define COV_PIN_COUNT 6
#define COV_TIMEOUT 60000
#define COV_INTERVAL 300
#define US_PIN 32
#define MAX_DISTANCE 200

void driverInit();
void driverLoop();
void onReceive(char* buffer,int length);
void displayText(char* text,int x=0,int y=25);
void displayAlert(char* text);
void displayTitle(char* text);
void displayIcons(int mask);
void redrawDisplay();
void ioInit();
void displayInit();
void clearText();
void clearTitle();
void clearDisplay();
bool checkErase();
void handleBuzzer();
