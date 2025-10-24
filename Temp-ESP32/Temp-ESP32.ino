#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// Configura tus datos Wi-Fi
const char* ssid = "TU_SSID";
const char* password = "TU_PASSWORD";

// URL de la API
const char* serverUrl = "https://e0b08479098d.ngrok-free.app/weather";

// Pines
#define DHTPIN 15
#define DHTTYPE DHT22
#define LEDPIN 2  // LED azul (puede variar según tu placa)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);

  // Conexión Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a Wi-Fi.");
}

void loop() {
  digitalWrite(LEDPIN, HIGH); // LED ON mientras lee

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error al leer el sensor.");
    digitalWrite(LEDPIN, LOW);
    delay(2000);
    return;
  }

  Serial.printf("Temp: %.2f °C | Humedad: %.2f %%\n", temperature, humidity);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authentication", "54654");  // Header personalizado

    // Crear JSON
    StaticJsonDocument<200> json;
    json["temperature"] = temperature;
    json["humidity"] = humidity;

    String requestBody;
    serializeJson(json, requestBody);

    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.printf("POST enviado. Código: %d\nRespuesta: %s\n", httpResponseCode, response.c_str());
    } else {
      Serial.printf("Error en POST: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
  } else {
    Serial.println("Wi-Fi desconectado. Intentando reconectar...");
    WiFi.reconnect();
  }

  digitalWrite(LEDPIN, LOW); // LED OFF

  // Dormir 5 minutos
  Serial.println("Esperando 5 minutos...");
  delay(5 * 60 * 1000);
}