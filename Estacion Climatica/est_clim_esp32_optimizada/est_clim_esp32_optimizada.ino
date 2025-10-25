#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>
#include <esp_sleep.h>

#define DHTPIN 15
#define DHTTYPE DHT22
#define LEDPIN 2

const char* ssid = "Cornerhouse";
const char* password = "baru50000";
const char* serverUrl = "https://octopus-app-3ypck.ondigitalocean.app/weather";
const char* authToken = "mTwcw4iFnKqC96Teur9mJxrpjb5XdiUDX1AVFRiLAyY";

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;

RTC_DATA_ATTR bool hasWiFiCredentials = false;
RTC_DATA_ATTR wifi_config_t savedWiFiConfig;

void setup() {
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  dht.begin();

  if (!bmp.begin(0x76)) {
    blinkError(4);
    goToSleep();
  }

  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
  if (wakeupReason == ESP_SLEEP_WAKEUP_TIMER && hasWiFiCredentials) {
    quickWiFiReconnect();
  } else {
    fullWiFiConnect();
  }

  if (WiFi.status() != WL_CONNECTED) {
    blinkError(2);  // WiFi error
    goToSleep();
  }

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;
  float altitude = bmp.readAltitude(1013.25);

  if (isnan(humidity) || isnan(temperature)) {
    blinkError(3);  // DHT22 error
    goToSleep();
  }

  bool success = false;
  const int maxRetries = 5;
  for (int attempt = 0; attempt < maxRetries && !success; ++attempt) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("authentication", authToken);

    StaticJsonDocument<256> json;
    json["Temperature"] = temperature;
    json["Humidity"] = humidity;
    json["Pressure"] = pressure;
    json["Altitude"] = altitude;

    String body;
    serializeJson(json, body);

    int code = http.POST(body);
    if (code == 200) {
      success = true;
      blinkSuccess();
    } else {
      blinkError(5);  // HTTP error
      delay(1000);
    }
    http.end();
  }

  goToSleep();
}

void loop() {}

void quickWiFiReconnect() {
  WiFi.disconnect(true);
  esp_wifi_set_config(WIFI_IF_STA, &savedWiFiConfig);
  WiFi.begin();
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 3000) {
    delay(100);
  }
}

void fullWiFiConnect() {
  WiFi.persistent(false);
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts++ < 20) {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    esp_wifi_get_config(WIFI_IF_STA, &savedWiFiConfig);
    hasWiFiCredentials = true;
  }
}

void goToSleep() {
  esp_sleep_enable_timer_wakeup(15 * 60 * 1000000ULL);
  esp_deep_sleep_start();
}

// BLINK GUIDE:
//1: OK
//2: WIFI ERROR
//3: DHT22 ERROR
//4: BMP280 ERROR
//5: HTTP ERROR

void blinkSuccess() {
  for (int i = 0; i < 1; ++i) {
    digitalWrite(LEDPIN, HIGH);
    delay(200);
    digitalWrite(LEDPIN, LOW);
    delay(200);
  }
}

void blinkError(int errorType) {
  for (int i = 0; i < errorType; ++i) {
    digitalWrite(LEDPIN, HIGH);
    delay(200);
    digitalWrite(LEDPIN, LOW);
    delay(200);
  }
}