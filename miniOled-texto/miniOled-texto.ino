#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  // Prueba colores
  display.setTextSize(2);
  display.setCursor(0,0);

  // Blanco (funciona en todas)
  display.setTextColor(SSD1306_WHITE);
  display.println("Temp: 23C");
  display.println("Hum:  90%");

  display.display();
}

void loop() {}
