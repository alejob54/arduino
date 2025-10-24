#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  delay(1000);
  Serial.println("Buscando dispositivos I2C...");

  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Dispositivo encontrado en 0x");
      Serial.println(address, HEX);
    }
  }
}

void loop() {}