#include <Wire.h>

void setup() {
  Wire.begin(); // Inicia I2C
  Serial.begin(9600);
  while (!Serial); // Espera a que el puerto esté listo (útil en algunos Arduinos)

  Serial.println("Iniciando escaneo I2C...");
}

void loop() {
  byte devicesFound = 0;

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Dispositivo I2C encontrado en dirección 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println();
      devicesFound++;
    }
  }

  if (devicesFound == 0) {
    Serial.println("No se encontraron dispositivos I2C.");
  } else {
    Serial.print("Total encontrados: ");
    Serial.println(devicesFound);
  }

  Serial.println("Escaneo finalizado.\n");
  delay(1000); // Espera 5 segundos antes de volver a escanear
}
