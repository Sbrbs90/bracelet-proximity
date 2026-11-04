// Bracelet A: finds bracelet B, connects over BLE, shows proximity on the LED strip.
// Has the button: 1 click = connect, 2 clicks = disconnect.
// Board: Arduino Nano 33 Sense / BLE. Libraries: ArduinoBLE, Adafruit NeoPixel.
// Wiring (Grove shield, 3.3V): button on D2, LED strip on A2.

#include <ArduinoBLE.h>
#include <Adafruit_NeoPixel.h>

const bool debug = false;   // true = print signal strength to the Serial Monitor

const char* svcUuid = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char* chrUuid = "19B10001-E8F2-537E-4F6C-D104768A1214";

const int buttonPin = 2;
const int ledPin = A2;
const int numLeds = 7;

Adafruit_NeoPixel strip(numLeds, ledPin, NEO_GRB + NEO_KHZ800);

const int farRssi = -80;    // the effect starts here
const int nearRssi = -50;   // maximum speed here
const int slowMs = 600;     // step time when far
const int fastMs = 60;      // step time when close

float smoothRssi = -100;
unsigned long lastRssi = 0;
unsigned long lastPrint = 0;
unsigned long lastStep = 0;
int pos = 0;
bool connected = false;

bool lastBtn = LOW;
int clicks = 0;
unsigned long lastClick = 0;

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);
  strip.begin();
  strip.setBrightness(100);
  strip.clear();
  strip.show();
  BLE.begin();
}

void loop() {
  BLE.scanForUuid(svcUuid);
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    BLE.stopScan();
    if (peripheral.connect() && peripheral.discoverAttributes()) {
      BLECharacteristic stateChar = peripheral.characteristic(chrUuid);
      smoothRssi = -100;   // start from "far"

      while (peripheral.connected()) {
        // read signal every 100 ms, discard invalid readings
        if (millis() - lastRssi > 100) {
          lastRssi = millis();
          int r = peripheral.rssi();
          if (r < -20 && r > -110) {
            smoothRssi = smoothRssi * 0.85 + r * 0.15;
          }
        }

        if (debug && millis() - lastPrint > 300) {
          lastPrint = millis();
          Serial.println(smoothRssi);
        }

        checkButton(stateChar);

        if (connected) {
          // LEDs stay on
        } else if (smoothRssi < farRssi) {
          strip.clear();
          strip.show();
        } else {
          approach();
        }
      }
    }
    // link lost: turn everything off
    connected = false;
    strip.clear();
    strip.show();
  }
}

void checkButton(BLECharacteristic &c) {
  bool s = digitalRead(buttonPin);
  if (s == HIGH && lastBtn == LOW && millis() - lastClick > 50) {
    clicks++;
    lastClick = millis();
  }
  lastBtn = s;

  // 400 ms without another click: decide what to do
  if (clicks > 0 && millis() - lastClick > 400) {
    if (clicks == 1 && !connected && smoothRssi > farRssi) {
      c.writeValue((byte)1);
      connectAnim();
    } else if (clicks >= 2 && connected) {
      c.writeValue((byte)0);
      disconnectAnim();
    }
    clicks = 0;
  }
}

void approach() {
  int interval = constrain(map((int)smoothRssi, farRssi, nearRssi, slowMs, fastMs), fastMs, slowMs);
  if (millis() - lastStep >= (unsigned long)interval) {
    lastStep = millis();
    strip.clear();
    strip.setPixelColor(pos, strip.Color(255, 0, 120));
    strip.show();
    pos = (pos + 1) % numLeds;
  }
}

void connectAnim() {
  connected = true;
  for (int k = 0; k < 2; k++) {
    setAll(255, 255, 255);
    delay(100);
    setAll(0, 0, 0);
    delay(100);
  }
  setAll(255, 0, 120);
}

void disconnectAnim() {
  connected = false;
  for (int i = numLeds - 1; i >= 0; i--) {
    strip.setPixelColor(i, 0);
    strip.show();
    delay(120);
  }
}

void setAll(int r, int g, int b) {
  for (int i = 0; i < numLeds; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}
