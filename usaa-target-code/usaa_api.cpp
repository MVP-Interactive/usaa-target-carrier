#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "usaa_api.h"

const char* hitUrlBase = "http://192.168.77.11:5300/api/sensor/";
const char* statusUrlBase = "http://192.168.77.11:5300/api/sensor/status/";
const char* configUrlBase = "http://192.168.77.11:5300/api/sensor/config/";

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

void APIPostStatus(uint8_t sensor_id) {

  DynamicJsonDocument doc(65536);
  doc["uptime"] = (float)(millis() / 1024.0f);
  doc["reboot_cause"] = esp_reset_reason();
  doc["id"] = sensor_id;
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
      Serial.println("T: " + String(t));
      ret_val.threshold = t;
      ret_val.threshold_is_set = true;
      semi_success = true;
    } else
      Serial.print("[HTTP] GET DIDN'T HAVE Threshold.\n");

    if (doc.containsKey("hit_wait")) {
      float t = doc["hit_wait"].as<float>();
      Serial.println("T: " + String(t));
      ret_val.hit_wait = t;
      ret_val.hit_wait_is_set = true;
      semi_success = true;
    } else
      Serial.print("[HTTP] GET DIDN'T HAVE hit_wait.\n");

    if (doc.containsKey("hit_flash")) {
      float t = doc["hit_flash"].as<float>();
      Serial.println("T: " + String(t));
      ret_val.hit_flash = t;
      ret_val.hit_flash_is_set = true;
      semi_success = true;
    } else
      Serial.print("[HTTP] GET DIDN'T HAVE hit_flash.\n");

    if (!semi_success) {
      Serial.println("PL: " + payload);
      Serial.print("Doc: ");
      serializeJson(doc, Serial);
    }

  } else
    Serial.print("[HTTP] GET FAILED.\n");

  return ret_val;
}
