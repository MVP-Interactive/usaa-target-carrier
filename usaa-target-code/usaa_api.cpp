// -*- mode: C++;  tab-width: 2; c-basic-offset: 2;  -*-

#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "usaa_api.h"

const char* hitUrlBase = "http://192.168.0.2:5300/api/sensor/";
const char* statusUrlBase = "http://192.168.0.2:5300/api/sensor/status/";
const char* configUrlBase = "http://192.168.0.2:5300/api/sensor/config/";

String hitUrl;
String statusUrl;
String configUrl;

void APIUpdateUrls(uint8_t sensor_id) {
  hitUrl = hitUrlBase + String(sensor_id);
  statusUrl = statusUrlBase + String(sensor_id);
  configUrl = configUrlBase + String(sensor_id);
  Serial.println("hitURL: " + hitUrl);
  Serial.println("statusURL: " + statusUrl);
  Serial.println("configURL: " + configUrl);
}

void APIPostHit() {

  HTTPClient http;
  http.begin(hitUrl);

  Serial.print("[HTTP] POST HIT...\n");
  // start connection and send HTTP header
  int httpCode = http.POST(NULL, 0);

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    String payload = http.getString();
    Serial.println("PL: " + payload);
    return;
  }
  Serial.print("[HTTP] POST FAILED.\n");
}

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

void APIPostStatus(uint8_t sensor_id, const String & rev) {

  DynamicJsonDocument doc(65536);
  doc["uptime"] = (float)(millis() / 1024.0f);
  doc["reboot_cause"] = RESET_REASONS[esp_reset_reason()];
  doc["id"] = sensor_id;
  doc["rev"] = rev;
  String json;
  serializeJson(doc, json);

  HTTPClient http;
  http.begin(statusUrl);
  http.addHeader("accept", "application/json");
  http.addHeader("Content-Type", "application/json");

  Serial.print("[HTTP] POST STATUS...\n");
  // start connection and send HTTP header
  int httpCode = http.POST(json);

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    String payload = http.getString();
    Serial.println("PL: " + payload);
    return;
  }
  Serial.print("[HTTP] POST FAILED.\n");
}

APIConfig APIGetConfig() {
  APIConfig ret_val;
  memset(&ret_val, 0, sizeof(ret_val));

  HTTPClient http;
  http.begin(configUrl);

  Serial.print("[HTTP] GET CONFIG...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    String payload = http.getString();

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    bool semi_success = false;  // Set true if one field can be read.
    if (doc.containsKey("id")) {
      uint8_t id = doc["id"].as<uint8_t>();
      Serial.println("Sid: " + String(id));
      semi_success = true;
    }
    if (doc.containsKey("threshold")) {
      float t = doc["threshold"].as<float>();
      Serial.println("threshold: " + String(t));
      ret_val.threshold = t;
      ret_val.threshold_is_set = true;
      semi_success = true;
    } else
      Serial.print("[HTTP] GET DIDN'T HAVE Threshold.\n");

    if (doc.containsKey("hit_wait")) {
      float t = doc["hit_wait"].as<uint32_t>();
      Serial.println("hit_wait: " + String(t));
      ret_val.hit_wait = t;
      ret_val.hit_wait_is_set = true;
      semi_success = true;
    } else
      Serial.print("[HTTP] GET DIDN'T HAVE hit_wait.\n");

    if (doc.containsKey("hit_flash")) {
      float t = doc["hit_flash"].as<uint32_t>();
      Serial.println("hit_flash: " + String(t));
      ret_val.hit_flash = t;
      ret_val.hit_flash_is_set = true;
      semi_success = true;
    } else
      Serial.print("[HTTP] GET DIDN'T HAVE hit_flash.\n");

    if (doc.containsKey("white_level")) {
      float t = doc["white_level"].as<uint8_t>();
      Serial.println("white_level: " + String(t));
      ret_val.white_level = t;
      ret_val.white_level_is_set = true;
      semi_success = true;
    } else
      Serial.print("[HTTP] GET DIDN'T HAVE white_level.\n");

    if (doc.containsKey("blink_interval")) {
      float t = doc["blink_interval"].as<uint32_t>();
      Serial.println("blink_interval: " + String(t));
      ret_val.blink_interval = t;
      ret_val.blink_interval_is_set = true;
      semi_success = true;
    } else
      Serial.print("[HTTP] GET DIDN'T HAVE blink_interval.\n");


    if (!semi_success) {
      Serial.println("PL: " + payload);
      Serial.print("Doc: ");
      serializeJson(doc, Serial);
    }

  } else
    Serial.print("[HTTP] GET FAILED.\n");

  return ret_val;
}
