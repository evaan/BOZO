#include <WiFi.h>
#include <esp_now.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

typedef struct depthStruct {
  int time;
  float pressure;
} depthStruct;
typedef struct enabledStruct {
  bool enabled;
} enabledStruct;
enabledStruct enabled;
depthStruct packet;

const uint8_t profilerAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //CHANGE TO PROFILERS MAC ADDRESS!
esp_now_peer_info_t profiler;

DynamicJsonDocument depthOverTime(4096);
String logs = "";

AsyncWebServer server(80);

void onReceive(const uint8_t *macAddress, const uint8_t *data, int length) {
  memcpy(&packet, data, sizeof(packet));
  time_t time = (time_t)packet.time/1000;
  char buffer[10];
  strftime(buffer, 10, "%H:%M:%S]", localtime(&time));
  logs += "EX00 - " + String(buffer) + " - " + String(packet.pressure) + " kPa - " + String(packet.pressure/9.81) +"m\n";
  int i = depthOverTime.size();
  depthOverTime[i][0] = packet.time;
  depthOverTime[i][1] = packet.pressure; 
}

void onSend(const uint8_t *macAddress, esp_now_send_status_t status) {
  if (status != 0) {
    //failure wah wah
  }
}

void setup() {
  Serial.begin(115200);
  if (!LittleFS.begin()) {
    Serial.println("Unable to start LittleFS!");
    return;
  }
  if (esp_now_init() != ESP_OK) {
    Serial.println("Unable to start ESP-NOW!");
    return;
  }
  esp_now_register_recv_cb(onReceive);
  esp_now_register_send_cb(onSend);
  memcpy(profiler.peer_addr, profilerAddress, 6);
  profiler.channel = 0;
  profiler.encrypt = false;
  if (esp_now_add_peer(&profiler) != ESP_OK) {
    Serial.println("Unable to initialize profiler!");
    return;
  }
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("EasternEdgeProfiler", "crazyasspassword73");
  WiFi.softAPConfig(IPAddress(192, 168, 0 , 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(LittleFS, "/index.html");});
  server.on("/jquery.min.js", [](AsyncWebServerRequest *request) {request->send(LittleFS, "/jquery.min.js");});
  server.on("/dygraph.min.js", [](AsyncWebServerRequest *request) {request->send(LittleFS, "/dygraph.min.js");});
  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request) {
    enabled.enabled = true;
    if (esp_now_send(profilerAddress, (uint8_t *) &enabled, sizeof(enabled)) != ESP_OK) {
      Serial.println("Unable to send packet to profiler!");
    } 
  });
  server.on("/data", [](AsyncWebServerRequest *request) {
    String tmp;
    serializeJson(depthOverTime, tmp);
    request->send(200, tmp);
  });
  server.on("/logs", [](AsyncWebServerRequest *request) {request->send(200, logs);});
}

void loop() {}