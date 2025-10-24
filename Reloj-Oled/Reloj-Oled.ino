#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long previousMillis = 0;
int seconds = 0, minutes = 0, hours = 12;

// Muestra la hora en formato HH:MM:SS
void showTime() {
  char buffer[9];
  sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);

  // Efecto de desvanecimiento: borra con gris (simulado)
  for (int fade = 0; fade < 3; fade++) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.print(buffer);
    display.display();
    delay(30);
  }
}

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 20);
  showTime();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    seconds++;

    if (seconds >= 60) {
      seconds = 0;
      minutes++;
    }
    if (minutes >= 60) {
      minutes = 0;
      hours++;
    }
    if (hours >= 24) {
      hours = 0;
    }

    showTime();  // Actualiza y muestra la hora con animación
  }
}
