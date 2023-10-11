// -*- mode: C++;  tab-width: 2; c-basic-offset: 2;  -*-

#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <Client.h>
#include <ETH.h>

#include <esp_task_wdt.h>
#include "esp_system.h"

#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>    // not used in this demo but required!
#include <Adafruit_NeoPixel.h>  //  Library that provides NeoPixel functions

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <CircularBuffer.h>
#include <CRC.h>

#include "usaa_api.h"

const String TAG = "rev_7";

#define WDT_TIMEOUT 120  // define a 1 minute WDT (Watch Dog Timer)

// 99 is invalid.  1 is normal power on. 6 is task watchdog. For others
// see: https://github.com/espressif/esp-idf/blob/272b4091f1f1ff169c84a4ee6b67ded4a005a8a7/components/esp_system/include/esp_system.h#L38
int BootReason = 99;

enum LedState {
  OFF,
  REGULAR,
  HIT,
  DISCONNECTED
};

enum HTTPMsg {
  HTTP_HIT,
  HTTP_STATUS,
  HTTP_CFG
};
uint8_t hit_thresh = 19;
// We don't want to do the sqrt part of the magnitudes, so we square the value to
// compare against.
float hit_thresh_sq = hit_thresh * hit_thresh;

long long lastHit = 0;
uint32_t hit_wait = 500;    //How long to enforce no hits after a hit, in ms
uint32_t hit_flash = 5000;  // How long to strobe LEDs in ms
uint8_t white_level = 191;
uint16_t blink_interval = 200;

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

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// i2c
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

CircularBuffer<float, 2000> sample_history;

long long lastStatus = 0;  // ms
long long lastConfig = 0;

const uint32_t statusInterval = 60000;  // ms
const uint32_t configInterval = 60000;

const String RESET_REASONS[] = {
    "ESP_RST_UNKNOWN",    //!< Reset reason can not be determined
    "ESP_RST_POWERON",    //!< Reset due to power-on event
    "ESP_RST_EXT",        //!< Reset by external pin (not applicable for ESP32)
    "ESP_RST_SW",         //!< Software reset via esp_restart
    "ESP_RST_PANIC",      //!< Software reset due to exception/panic
    "ESP_RST_INT_WDT",    //!< Reset (software or hardware) due to interrupt watchdog
    "ESP_RST_TASK_WDT",   //!< Reset due to task watchdog
    "ESP_RST_WDT",        //!< Reset due to other watchdogs
    "ESP_RST_DEEPSLEEP",  //!< Reset after exiting deep sleep mode
    "ESP_RST_BROWNOUT",   //!< Brownout reset (software or hardware)
    "ESP_RST_SDIO",       //!< Reset over SDIO
    "ESP_RST_USB",        //!< Reset by USB peripheral
    "ESP_RST_JTAG",       //!< Reset by JTAG
};

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

void setupNetwork() {
  WiFi.onEvent(WiFiEvent);

  ETH.begin();
  Serial.print("ETH MAC: ");
  Serial.println(ETH.macAddress());

  uint64_t chipid = ESP.getEfuseMac();  // The chip ID is essentially its MAC address(length: 6 bytes).

  uint8_t* mac = (uint8_t*)&chipid;
  sensor_id = (calcCRC8(mac, 8) & 0x7f) + 20;
  ip = IPAddress(192, 168, 77, sensor_id);
  APIUpdateUrls(sensor_id);

  ETH.config(ip, gw, subnet, gw, gw);
}

void setup() {
  Serial.begin(115200);

  while (!Serial) {
    delay(1);  // will pause Zero, Leonardo, etc until serial console opens
  }

  Serial.println("\n Starting...\n");
  Serial.println("\n Rev " + TAG + " ...\n");
  Serial.flush();

  strip.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();   // Turn OFF all pixels ASAP
  //strip.setBrightness(100); // Set BRIGHTNESS to about 1/5 (max = 255)

  Serial.println("MVP Hit Sensor ...");
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
  Serial.println(RESET_REASONS[BootReason]);
}

void writeLEDs(LedState state) {
  switch (state) {
    case OFF:
      strip.clear();
      break;
    case HIT:
      {
        long long now = millis();
        int32_t diff = now - lastHit;
        int blinkState = (diff / blink_interval) & 0x00000001;

        if (blinkState) {
          for (int i = 0; i < strip.numPixels(); i++) {
            if (i < LED_SPLIT) {
              strip.setPixelColor(i, 0, 0, 255);
            } else {
              strip.setPixelColor(i, white_level, white_level, white_level);
            }
          }
        } else {
          for (int i = 0; i < strip.numPixels(); i++) {
            if (i < LED_SPLIT) {
              strip.setPixelColor(i, white_level, white_level, white_level);
            } else {
              strip.setPixelColor(i, 0, 0, 255);
            }
          }
        }
        strip.show();  //  Update strip to match
                       //  Pause for a moment
      }
      break;
    case REGULAR:
      //remember NEO_GRB
      for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
        if (i < LED_SPLIT) {
          strip.setPixelColor(i, 0, 0, 255);
        } else {
          strip.setPixelColor(i, white_level, white_level, white_level);  //  Set pixel's color (in RAM)
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

void checkStatusConfig() {
  unsigned long now = millis();

  if (lastStatus == 0 || now > lastStatus + statusInterval) {
    APIPostStatus(sensor_id);
    lastStatus = now;
  }

  if (lastConfig == 0 || now > lastConfig + configInterval) {
    APIConfig config = APIGetConfig();
    if (config.threshold_is_set && config.threshold > 1) {  // No way a threshhold of less than 1g is valid
      hit_thresh = config.threshold;
      hit_thresh_sq = hit_thresh * hit_thresh;
      Serial.println("New threshold: " + String(hit_thresh));
    }

    if (config.hit_wait_is_set) {
      hit_wait = config.hit_wait;
      Serial.println("New hit_wait: " + String(hit_wait));
    }

    if (config.hit_flash_is_set) {
      hit_flash = config.hit_flash;
      Serial.println("New hit_flash: " + String(hit_flash));
    }

    if (config.blink_interval_is_set) {
      blink_interval = config.blink_interval;
      Serial.println("New hit_flash: " + String(blink_interval));
    }

    if (config.white_level_is_set) {
      white_level = config.white_level;
      Serial.println("New white_level: " + String(white_level));
    }

    lastConfig = now;
  }
}

void debugInfo(float magnitude_sq, sensors_vec_t a) {
  Serial.print("M: ");
  Serial.print(magnitude_sq);
  Serial.print(", HT: ");
  Serial.print(hit_thresh_sq);
  Serial.print(", A_X: ");
  Serial.print(a.x);
  Serial.print(", A_Y: ");
  Serial.print(a.y);
  Serial.print(", A_Z: ");
  Serial.println(a.z);
}

void loop() {
  esp_task_wdt_reset();  // Added to repeatedly reset the Watch Dog Timer

  checkStatusConfig();

  lsm.read(); /* ask it to read in the data */

  /* Get a new sensor event */
  sensors_event_t a, m, g, temp;

  lsm.getEvent(&a, &m, &g, &temp);

  float magnitude_sq = a.acceleration.x * a.acceleration.x
                       + a.acceleration.y * a.acceleration.y
                       + a.acceleration.z * a.acceleration.z;
  sample_history.push(magnitude_sq);

  // 1 Gravity Squared is 96.04
  if (magnitude_sq > 110) {
    debugInfo(magnitude_sq, a.acceleration);
  }

  bool isHit = false;
  long long now = millis();

  if (lastHit && now < lastHit + hit_wait) {
    isHit = true;
  } else if (magnitude_sq > hit_thresh_sq) {
    isHit = true;
    lastHit = now;
    if (eth_connected) {
      APIPostHit();
    }
  }

  if (lastHit && now < lastHit + hit_flash) {
    isHit = true;
  }

  if (!eth_connected)
    writeLEDs(DISCONNECTED);
  else if (isHit)
    writeLEDs(HIT);
  else
    writeLEDs(REGULAR);

  delay(5);
}
