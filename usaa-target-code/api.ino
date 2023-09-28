#include <ArduinoJson.h>

const char* hitUrlBase = "http://192.168.77.11:5300/api/sensor/";
const char* statusUrlBase = "http://192.168.77.11:5300/api/sensor/status/";
const char* configUrlBase = "http://192.168.77.11:5300/api/sensor/config/";

String hitUrl;
String statusUrl;
String configUrl;

void updateUrls(uint8_t sensor_id) {
  hitUrl = hitUrlBase + String(sensor_id);
  statusUrl = statusUrlBase + String(sensor_id);
  configUrl = configUrlBase + String(sensor_id);
  Serial.println("hitURL: " + hitUrl);
  Serial.println("statusURL: " + statusUrl);
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

float APIGetConfig() {

  HTTPClient http;
  http.begin(hitUrl);

  Serial.print("[HTTP] GET CONFIG...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, http.getStream());
    uint8_t id = doc["Id"].as<uint8_t>();
    float t = doc["Threshold"].as<float>();
    Serial.println("Sid: " + String(id));
    Serial.println("T: " + String(t));

    return t;
  }
  Serial.print("[HTTP] GET FAILED.\n");
  return 0;
}
