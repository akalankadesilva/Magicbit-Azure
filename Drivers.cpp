#include "Drivers.h"
char textBuffer[96];
char titleBuffer[16];
int iconMask = 0;
int lastBaud = 4;
unsigned long baud[5] = {9600, 19200, 38400, 57600, 115200};
unsigned long lastAlert = 0;
bool isDisplayAlert = false;
char txBuffer[64];
int txLength;
int interruptPins[INT_PIN_COUNT];
int lastInterruptVals[INT_PIN_COUNT];
int covPins[COV_PIN_COUNT];
int covThresholds[COV_PIN_COUNT];
int lastCovVals[COV_PIN_COUNT];
unsigned long lastCovTimestamp = 0;
unsigned long lastCovQuery[COV_PIN_COUNT];
unsigned long lastBuzz = 0;
int buzzDuration = 1000;
Servo servo;
static const unsigned char PROGMEM onlineBMP[] =
{ 
  0x00, 0x00, 0x07, 0xf0, 0x1f, 0xf8, 0x78, 0x1e, 0xe0, 0x06, 0xc7, 0xe2, 0x0f, 0xf0, 0x1c, 0x38,
  0x18, 0x10, 0x01, 0x80, 0x03, 0xc0, 0x03, 0x80, 0x00, 0x00
};


Adafruit_SSD1306 display(128, 64);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel LED(1, 32, NEO_RGB + NEO_KHZ800);
NewPing sonar(US_PIN, US_PIN, MAX_DISTANCE);

void driverInit() {

  displayInit();
  ioInit();
  for (int i = 0; i < INT_PIN_COUNT; i++) {
    interruptPins[i] = -1;
  }
  for (int i = 0; i < COV_PIN_COUNT; i++) {
    covPins[i] = -1;
  }

}

bool checkErase() {
  return digitalRead(PB_LEFT) == 0;
}


void ioInit() {
  pinMode(PB_RIGHT, INPUT);
  pinMode(PB_LEFT , INPUT);
  pinMode(LDR, INPUT);
  pinMode(POT, INPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);
  tone(BUZZER, 1000);
  delay(500);
  noTone(BUZZER);

}

void displayInit() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();

}
void clearDisplay() {
  display.clearDisplay();
  display.display();
}

void redrawDisplay() {



  display.clearDisplay();


  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(1, 0);
  display.println(titleBuffer);

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.println(textBuffer);

  if ((iconMask & 1) > 0)
    display.drawBitmap(113, 0, onlineBMP, 15, 15, 1);
  else if ((iconMask & 2) > 0) {
    display.setTextSize(2);
    display.setCursor(113, 0);
    display.println("X");
  }
  display.display();
}
void clearText() {
  memset(textBuffer, 0, sizeof(textBuffer));
  redrawDisplay();
}
void clearTitle() {
  memset(titleBuffer, 0, sizeof(titleBuffer));
  redrawDisplay();
}
void displayText(char* text, int x, int y) {

  strcpy(textBuffer, text);
  redrawDisplay();
}
void displayAlert(char* text) {
  lastAlert = millis();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(1, 0);
  display.println("Alert");

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.println(text);
  display.display();
  isDisplayAlert = true;
}
void displayTitle(char* text) {
  strcpy(titleBuffer, text);
  redrawDisplay();
}
void displayIcons(int mask) {
  iconMask = mask;
}
void driverLoop() {
  int t = 0;
  if (millis() - lastAlert > ALERT_TIMEOUT || millis() < lastAlert) {
    redrawDisplay();
    isDisplayAlert = false;
  }
  if (millis() - lastCovTimestamp > COV_INTERVAL || millis() < lastCovTimestamp) {
    lastCovTimestamp = millis();
    for (int i = 0; i < INT_PIN_COUNT; i++) {
      if (interruptPins[i] != -1) {
        pinMode(interruptPins[i], INPUT);
        t = digitalRead(interruptPins[i]);
        if (t != lastInterruptVals[i]) {
          lastInterruptVals[i] = t;
          txBuffer[txLength] = 0x01;
          txBuffer[txLength + 1] = 0x90;
          txBuffer[txLength + 2] = interruptPins[i];
          txBuffer[txLength + 3] = t;
          txLength += 4;
        }
      }
    }
    for (int i = 0; i < COV_PIN_COUNT; i++) {
      if (covPins[i] != -1) {
        if (millis() - lastCovQuery[i] > COV_TIMEOUT) {
          covPins[i] = -1;
          continue;
        }
        t = analogRead(covPins[i]);
        if (abs(t - lastCovVals[i]) > covThresholds[i]) {
          lastCovVals[i] = t;
          txBuffer[txLength] = 0x01;
          txBuffer[txLength + 1] = 0xE0;
          txBuffer[txLength + 2] = covPins[i];
          txBuffer[txLength + 3] = (t >> 8);
          txBuffer[txLength + 4] = (t & 0xFF);
          txLength += 5;
        }
      }
    }
  }
  t = Serial.available();
  if (t > 0) {
    txBuffer[txLength] = 0x01;
    txBuffer[txLength + 1] = 0xF0;
    txBuffer[txLength + 2] = 0x63;
    for (int i = 0; i < t; i++) {
      txBuffer[txLength + 3 + i] = Serial.read();
    }
    txBuffer[txLength + 3 + t] = 0xF7;
    txLength += (4 + t);
  }

  handleBuzzer();
}

void handleBuzzer() {
  if (millis() - lastBuzz > buzzDuration || millis() < lastBuzz) {
    if (lastBuzz == 0)
      return;
    lastBuzz = 0;
    noTone(BUZZER);
  }
}
void onReceive(char* buffer, int length) {
  if (buffer[0] != 0x01)
    return;
  int t = 1;
  int i = 0;
  int j = 0;
  int k = 0;
  float f1 = 0.0;
  float f2 = 0.0;
  int lastT = t;
  char* c;
  int freq;
  while (t < length) {

    lastT = t;
    switch (buffer[t]) {
      case 0xF4:
        switch (buffer[t + 2]) {
          case 0x00:
            pinMode(buffer[t + 1], INPUT);
            break;
          case 0x01:
            pinMode(buffer[t + 1], OUTPUT);
            break;
          case 0x03:
            pinMode(buffer[t + 1], OUTPUT);
            break;
        };
        t += 3;
        break;
      case 0x90:
        digitalWrite(buffer[t + 1], buffer[t + 2]);
        t += 3;
        break;
      case 0xD0:
        i = digitalRead(buffer[t + 1]);
        txBuffer[txLength] = 0x01;
        txBuffer[txLength + 1] = 0x90;
        txBuffer[txLength + 2] = buffer[t + 1];
        txBuffer[txLength + 3] = i;
        txLength += 4;
        t += 2;
        break;
      case 0xD1:
        for (i = 0; i < INT_PIN_COUNT; i++) {
          if (interruptPins[i] == buffer[t + 1])
            break;
          if (i == INT_PIN_COUNT - 1) {
            for (int j = 0; j < INT_PIN_COUNT; j++) {
              if (interruptPins[j] == -1) {
                interruptPins[j] = buffer[t + 1];
                break;
              }
              if (j == INT_PIN_COUNT - 1) {
                for (int k = 0; k < INT_PIN_COUNT - 1; k++) {
                  interruptPins[k] = interruptPins[k + 1];
                }
                interruptPins[INT_PIN_COUNT - 1] = buffer[t + 1];
              }
            }
          }
        }
        t += 2;
        break;
      case 0xE0:
        i = buffer[t + 2];
        i = (i << 8);
        i += buffer[t + 3];
        i = i / 4;
        analogWrite(buffer[t + 1], i);
        t += 4;
        break;
      case 0xC0:
        i = analogRead(buffer[t + 1]);
        txBuffer[txLength] = 0x01;
        txBuffer[txLength + 1] = 0xE0;
        txBuffer[txLength + 2] = buffer[t + 1];
        txBuffer[txLength + 3] = (i >> 8);
        txBuffer[txLength + 4] = (i & 0xFF);
        txLength += 5;
        t += 2;
        break;
      case 0xC1:
        for (i = 0; i < COV_PIN_COUNT; i++) {

          if (covPins[i] == buffer[t + 1]) {
            covThresholds[i] = buffer[t + 2];
            lastCovQuery[i] = millis();
            break;
          }
          if (i == COV_PIN_COUNT - 1) {
            for (int j = 0; j < COV_PIN_COUNT; j++) {
              if (covPins[j] == -1) {
                covPins[j] = buffer[t + 1];
                covThresholds[j] = buffer[t + 2];
                lastCovQuery[j] = millis();
                break;
              }
              if (j == COV_PIN_COUNT - 1) {
                for (int k = 0; k < COV_PIN_COUNT - 1; k++) {
                  covPins[k] = covPins[k + 1];
                }
                covPins[COV_PIN_COUNT - 1] = buffer[t + 1];
                covThresholds[COV_PIN_COUNT - 1] = buffer[t + 2];
                lastCovQuery[COV_PIN_COUNT - 1] = millis();
              }
            }
          }
        }
        t += 3;
        break;
      case 0xF0:
        switch (buffer[t + 1]) {
          case 0x62:
            if (lastBaud != buffer[t + 2]) {
              Serial.end();
              Serial.begin(baud[buffer[t + 2]]);
              delay(100);
              lastBaud = buffer[t + 2];
            }
            for (i = 0; i < buffer[t + 3]; i++) {
              Serial.write(buffer[t + 4 + i]);
            }
            t += (5 + buffer[t + 3]);
            break;
          case 0x64:
            c = strchr(buffer + t + 3, '|');
            i = (int)(c - buffer) - t - 3;
            strncpy(titleBuffer, buffer + t + 3, i);
            strncpy(textBuffer, c + 1, buffer[t + 2] - i - 1);
            textBuffer[buffer[t + 2] - i - 1] = 0;
            titleBuffer[i] = 0;
            Serial.println(titleBuffer);
            Serial.println(textBuffer);
            redrawDisplay();
            t += (4 + buffer[t + 2]);
            break;
          case 0x65:
            lastBuzz = millis();
            buzzDuration = buffer[t + 4];
            buzzDuration = buzzDuration << 8;
            buzzDuration += buffer[t + 5];
            freq = buffer[t + 2];
            freq = freq << 8;
            freq += buffer[t + 3];
            tone(BUZZER, freq);
            t += 7;
            break;
          case 0x66:
            dht.begin(buffer[t + 2]);
            Serial.println("DHT Pin: "); Serial.println(buffer[t + 2]);
            f1 = dht.readHumidity(); Serial.println(f1);
            f2 = dht.readTemperature(); Serial.println(f2);
            i = (int)(f1 * 10);
            j = (int)(f2 * 10);
            txBuffer[txLength] = 0x01;
            txBuffer[txLength + 1] = 0xF0;
            txBuffer[txLength + 2] = 0x66;
            txBuffer[txLength + 3] = i >> 8;
            txBuffer[txLength + 4] = i & 0xFF;
            txBuffer[txLength + 5] = j >> 8;
            txBuffer[txLength + 6] = j & 0xFF;
            txBuffer[txLength + 1] = 0xF7;
            txLength += 8;
            t += 4;
            break;

          case 0x67:
            pinMode(buffer[t + 2], OUTPUT);
            digitalWrite(buffer[t + 2], LOW);
            LED.setPin(buffer[t + 2]);
            Serial.print("LED Pin: "); Serial.println(buffer[t + 2]);
            i = buffer[t + 3];
            for (j = 0; j < i; j++) {
              Serial.print("Color: "); Serial.print(buffer[3 * j + 4]); Serial.print(" "); Serial.print(buffer[3 * j + 5]); Serial.print(" "); Serial.println(buffer[3 * j + 6]);
              LED.setPixelColor(j, buffer[3 * j + 4], buffer[3 * j + 5], buffer[3 * j + 6]);
            }
            LED.show();
            t += (3 * buffer[t + 3] + 4);
            break;
          case 0x68:
            i = sonar.ping_cm();
            Serial.println("Sonar: "); Serial.println(i);

            txBuffer[txLength] = 0x01;
            txBuffer[txLength + 1] = 0xF0;
            txBuffer[txLength + 2] = 0x68;
            txBuffer[txLength + 3] = i >> 8;
            txBuffer[txLength + 4] = i & 0xFF;
            txBuffer[txLength + 5] = 0xF7;
            txLength += 6;
            t += 3;
            break;
          case 0x6F:
            servo.attach(buffer[t+2]);
            servo.write(buffer[t+3]);
            t+=5;

            break;


        };
        break;
    };
    if (t == lastT)
      break;
  }

}
