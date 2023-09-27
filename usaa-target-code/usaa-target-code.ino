#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>    // not used in this demo but required!
#include <Adafruit_NeoPixel.h>  //  Library that provides NeoPixel functions

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>
#include <Client.h>
#include <ETH.h>

#include <esp_task_wdt.h>
#include "esp_system.h"


#define WDT_TIMEOUT 120  // define a 3 seconds WDT (Watch Dog Timer)
int BootReason = 99;

enum LedState {
  OFF,
  REGULAR,
  HIT,
  DISCONNECTED
};

enum HTTPMsg {
  HTTP_HIT,

};

const uint8_t HIT_THRESH = 12;

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
const uint8_t LED_PIN = 4;

// How many NeoPixels are attached to the Arduino?
const uint8_t LED_COUNT = 50;
const uint8_t LED_SPLIT = 18;

uint8_t sensor_id;
IPAddress ip(192, 168, 77, 21);
const IPAddress gw(192, 168, 77, 1);
const IPAddress subnet(255, 255, 255, 0);
const String hitUrl = "http://192.168.77.11:5301/api/player/";

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// i2c
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

void setupSensor() {
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);

  // 2.) Set the magnetometer sensitivity
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);

  // 3.) Setup the gyroscope
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
}

static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

void testClient(HTTPMsg msg) {

  HTTPClient http;
  http.begin(hitUrl);

  Serial.print("[HTTP] POST...\n");
  // start connection and send HTTP header
  int httpCode = http.POST(NULL, 0);

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  }
}

void setupNetwork() {
  WiFi.onEvent(WiFiEvent);

  ETH.begin();
  //ETH.macAddress()
  Serial.print("ETH MAC: ");
  Serial.println(ETH.macAddress());
  uint64_t chipid = ESP.getEfuseMac();  // The chip ID is essentially its MAC address(length: 6 bytes).
  uint8_t addr = ((uint32_t)chipid) % 230;
  sensor_id = addr + 20;
  ip = IPAddress(192, 168, 77, sensor_id);

  ETH.config(ip, gw, subnet, gw, gw);
}

void setup() {
  Serial.begin(115200);

  while (!Serial) {
    delay(1);  // will pause Zero, Leonardo, etc until serial console opens
  }

  Serial.println("\n Starting...\n");
  Serial.flush();

  strip.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();   // Turn OFF all pixels ASAP
  //strip.setBrightness(100); // Set BRIGHTNESS to about 1/5 (max = 255)

  Serial.println("LSM9DS1 data read demo");
  delay(1);

  Wire.begin(13, 16);  // init I2C on the respective pins

  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin()) {
    Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
    Serial.flush();
    while (1)
      ;
  }
  //Serial.println("Found LSM9DS1 9DOF");

  // helper to just set the default scaling we want, see above!
  setupSensor();
  setupNetwork();

  esp_task_wdt_init(WDT_TIMEOUT, true);  // enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);                // add current thread to WDT watch
  BootReason = esp_reset_reason();
  Serial.print("Boot reason:");
  Serial.println(BootReason);
}

void writeLEDs(LedState state) {
  switch (state) {
    case OFF:
      strip.clear();
      break;
    case HIT:
    case REGULAR:
      //remember NEO_GRB
      for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
        if (i < LED_SPLIT) {
          strip.setPixelColor(i, 0, 0, 255);
        } else {
          strip.setPixelColor(i, 255, 255, 255);  //  Set pixel's color (in RAM)
        }
      }
      strip.show();  //  Update strip to match
                     //  Pause for a moment

      break;
    case DISCONNECTED:
      for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
        strip.setPixelColor(i, 0, 255, 0);
      }
      strip.show();  //  Update strip to match
                     //  Pause for a moment
      break;
  }
}

void loop() {
  esp_task_wdt_reset();  // Added to repeatedly reset the Watch Dog Timer

  if (eth_connected)
    writeLEDs(REGULAR);
  else
    writeLEDs(DISCONNECTED);

  lsm.read(); /* ask it to read in the data */

  /* Get a new sensor event */
  sensors_event_t a, m, g, temp;

  lsm.getEvent(&a, &m, &g, &temp);

  if (a.acceleration.x > 10 || a.acceleration.y > 10 || a.acceleration.z > 10) {

    Serial.print("A_X: ");
    Serial.print(a.acceleration.x);
    Serial.print(", ");
    Serial.print("A_Y: ");
    Serial.print(a.acceleration.y);
    Serial.print(", ");
    Serial.print("A_Z: ");
    Serial.print(a.acceleration.z);
    Serial.println("");
  }

  int rc = 30, gc = 0, bc = 100;  //  Red, green and blue intensity to display

  if (a.acceleration.x > HIT_THRESH || a.acceleration.y > HIT_THRESH || a.acceleration.z > HIT_THRESH) {
    rc = 200, gc = 0, bc = 0;
    if (eth_connected) {
      testClient(HTTP_HIT);
    }
  }
  delay(10);
}
