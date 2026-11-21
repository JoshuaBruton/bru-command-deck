#include <Arduino.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

static const uint32_t WIFI_RETRY_MS = 12000;
static const uint32_t HEALTH_CHECK_MS = 15000;
static const uint32_t DRAW_INTERVAL_MS = 350;

const char* items[] = {"Dashboard", "Network", "Chappie", "Tools"};
constexpr int ITEM_COUNT = sizeof(items) / sizeof(items[0]);
int selected = 0;
bool inPage = false;
bool serverOnline = false;
int healthHttpCode = 0;
uint32_t lastWifiAttempt = 0;
uint32_t lastHealthCheck = 0;
uint32_t lastDraw = 0;
bool forceHealthCheck = true;

void drawText(int x, int y, const String& text, uint16_t color = WHITE, int size = 2) {
  M5.Lcd.setTextColor(color, TFT_BLACK);
  M5.Lcd.setTextSize(size);
  M5.Lcd.setCursor(x, y);
  M5.Lcd.print(text);
}

void drawScreen() {
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.fillRect(0, 0, 320, 29, 0x11C7);
  M5.Lcd.setTextColor(TFT_CYAN, 0x11C7);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(9, 7);
  M5.Lcd.print("BRu COMMAND v0.1");

  if (!inPage) {
    for (int i = 0; i < ITEM_COUNT; ++i) {
      int y = 43 + i * 37;
      if (i == selected) M5.Lcd.fillRoundRect(7, y - 4, 305, 32, 5, 0x22C9);
      M5.Lcd.setTextColor(i == selected ? TFT_YELLOW : TFT_WHITE,
                          i == selected ? 0x22C9 : TFT_BLACK);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setCursor(16, y + 2);
      M5.Lcd.print(i == selected ? "> " : "  ");
      M5.Lcd.print(items[i]);
    }
  } else {
    drawText(12, 45, items[selected], TFT_CYAN);
    if (selected == 0 || selected == 1) {
      drawText(12, 85, String("WiFi: ") + (WiFi.status() == WL_CONNECTED ? "CONNECTED" : "OFFLINE"),
               WiFi.status() == WL_CONNECTED ? TFT_GREEN : TFT_RED);
      if (WiFi.status() == WL_CONNECTED) {
        drawText(12, 113, String("IP: ") + WiFi.localIP().toString(), TFT_WHITE, 1);
        drawText(12, 133, String("RSSI: ") + WiFi.RSSI() + " dBm", TFT_WHITE, 2);
      }
      drawText(12, 164, String("Psydekik: ") + (serverOnline ? "ONLINE" : "OFFLINE"),
               serverOnline ? TFT_GREEN : TFT_RED);
      drawText(12, 190, String("HTTP: ") + healthHttpCode, TFT_LIGHTGREY, 1);
    } else if (selected == 2) {
      drawText(12, 92, "Chappie: coming soon", TFT_YELLOW, 2);
      drawText(12, 129, "No AI key on device", TFT_LIGHTGREY, 1);
    } else {
      drawText(12, 92, "Tools: coming soon", TFT_YELLOW, 2);
    }
  }
  M5.Lcd.fillRect(0, 211, 320, 29, 0x11C7);
  M5.Lcd.setTextColor(TFT_WHITE, 0x11C7);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(14, 222);
  M5.Lcd.print(inPage ? "A:Refresh" : "A:Up");
  M5.Lcd.setCursor(121, 222);
  M5.Lcd.print(inPage ? "B:Back" : "B:Open");
  M5.Lcd.setCursor(224, 222);
  M5.Lcd.print(inPage ? "C:Back" : "C:Down");
}

void checkHealth() {
  serverOnline = false;
  healthHttpCode = 0;
  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClient client;
  HTTPClient http;
  http.setConnectTimeout(1800);
  http.setTimeout(2200);
  if (!http.begin(client, PSYDEKIK_HEALTH_URL)) {
    healthHttpCode = -1;
    return;
  }
  healthHttpCode = http.GET();
  serverOnline = healthHttpCode == HTTP_CODE_OK;
  http.end();
  Serial.printf("Health check HTTP %d\n", healthHttpCode);
}

void setup() {
  M5.begin();
  M5.Power.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWifiAttempt = millis();
  drawScreen();
}

void loop() {
  M5.update();
  const uint32_t now = millis();
  bool changed = false;
  if (WiFi.status() != WL_CONNECTED && now - lastWifiAttempt >= WIFI_RETRY_MS) {
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastWifiAttempt = now;
    changed = true;
  }
  if (forceHealthCheck || now - lastHealthCheck >= HEALTH_CHECK_MS) {
    forceHealthCheck = false;
    lastHealthCheck = now;
    checkHealth();
    changed = true;
  }
  if (!inPage) {
    if (M5.BtnA.wasPressed()) { selected = (selected + ITEM_COUNT - 1) % ITEM_COUNT; changed = true; }
    if (M5.BtnC.wasPressed()) { selected = (selected + 1) % ITEM_COUNT; changed = true; }
    if (M5.BtnB.wasPressed()) { inPage = true; changed = true; }
  } else {
    if (M5.BtnA.wasPressed()) { forceHealthCheck = true; changed = true; }
    if (M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) { inPage = false; changed = true; }
  }
  if (changed || now - lastDraw >= DRAW_INTERVAL_MS) {
    // Avoid unnecessary full-screen redraws when no information changed.
    if (changed) drawScreen();
    lastDraw = now;
  }
  delay(15);
}
