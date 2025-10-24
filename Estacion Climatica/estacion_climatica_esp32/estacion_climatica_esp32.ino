#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>
#include <esp_sleep.h>

// Pines y configuración
#define DHTPIN 15
#define DHTTYPE DHT22
#define LEDPIN 2

const char* ssid = "Cornerhouse";
const char* password = "baru50000";
const char* serverUrl = "https://octopus-app-3ypck.ondigitalocean.app/weather";
const char* authToken = "mTwcw4iFnKqC96Teur9mJxrpjb5XdiUDX1AVFRiLAyY";

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;

// Variables persistentes en RAM de bajo consumo
RTC_DATA_ATTR bool hasWiFiCredentials = false;
RTC_DATA_ATTR wifi_config_t savedWiFiConfig;

void setup() {
  Serial.begin(115200);
  delay(100);
  printMemoryStats();

  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  dht.begin();

  // Inicializar BMP280
  if (!bmp.begin(0x76)) {
    Serial.println("No se pudo encontrar el BMP280. Durmiendo...");
    goToSleep();
  }

  // Verificar si despertó de deep sleep
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

  if (wakeupReason == ESP_SLEEP_WAKEUP_TIMER && hasWiFiCredentials) {
    Serial.println("Deep sleep → reconexión rápida");
    quickWiFiReconnect();
  } else {
    Serial.println("Reinicio completo → conexión completa");
    fullWiFiConnect();
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No conectado a Wi-Fi. Durmiendo...");
    goToSleep();
  }

  // Leer datos de sensores
  digitalWrite(LEDPIN, HIGH);
  float humidity = dht.readHumidity();
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F; // en hPa
  float altitude = bmp.readAltitude(1013.25); // requiere presión al nivel del mar
  digitalWrite(LEDPIN, LOW);

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error al leer sensores. Durmiendo...");
    goToSleep();
  }

  Serial.printf("Temp: %.2f °C | Humedad: %.2f %% | Presión: %.2f hPa | Altitud: %.2f m\n",
                temperature, humidity, pressure, altitude);

  // Intentar enviar POST (máx. 5 veces)
  bool success = false;
  const int maxRetries = 5;
  for (int attempt = 1; attempt <= maxRetries && !success; ++attempt) {
    Serial.printf("POST intento #%d...\n", attempt);
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
      String response = http.getString();
      Serial.printf("POST exitoso (%d): %s\n", code, response.c_str());
      success = true;
      blinkSuccess();
    } 
    else {
      blinkError();
      Serial.printf("POST fallido: %s\n", http.errorToString(code).c_str());
      delay(1000); // Espera antes de reintentar
    }
    http.end();
  }

  printMemoryStats();
  goToSleep();
}

void loop() {
  // No se usa. Todo está en setup() por el ciclo de deep sleep.
}

void quickWiFiReconnect() {
  WiFi.disconnect(true);  // Limpia estado
  esp_wifi_set_config(WIFI_IF_STA, &savedWiFiConfig);
  WiFi.begin();
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 3000) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
}

void fullWiFiConnect() {
  WiFi.persistent(false);
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts++ < 20) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    esp_wifi_get_config(WIFI_IF_STA, &savedWiFiConfig);
    hasWiFiCredentials = true;
    Serial.println("Wi-Fi conectado y config guardada.");
  }
}

void goToSleep() {
  Serial.println("Entrando en deep sleep 15 minutos...");
  esp_sleep_enable_timer_wakeup(15 * 60 * 1000000ULL); // 15 min en microsegundos
  esp_deep_sleep_start();
}

void printMemoryStats() {
  size_t totalHeap = ESP.getHeapSize();
  size_t freeHeap = ESP.getFreeHeap();
  size_t minHeap = ESP.getMinFreeHeap();
  float freeHeapPercent = ((float)freeHeap / totalHeap) * 100.0;

  Serial.println();
  Serial.print("Heap total: ");
  Serial.print(totalHeap);
  Serial.print(" | Libre: ");
  Serial.print(freeHeap);
  Serial.print(" (");
  Serial.print(freeHeapPercent, 2);
  Serial.print("%) | Mínimo histórico: ");
  Serial.print(minHeap);
  Serial.print(" | CPU MHz: ");
  Serial.println(getCpuFrequencyMhz());
}

void blinkSuccess() {
  for (int i = 0; i < 3; ++i) {
    digitalWrite(LEDPIN, HIGH);
    delay(200);
    digitalWrite(LEDPIN, LOW);
    delay(200);
  }
}

void blinkError() {
  for (int i = 0; i < 1; ++i) {
    digitalWrite(LEDPIN, HIGH);
    delay(200);
    digitalWrite(LEDPIN, LOW);
    delay(200);
  }
}