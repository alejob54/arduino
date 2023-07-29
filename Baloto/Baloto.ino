#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int switchPin = 6;
int switchState = 0;
int prevSwitchState = 0;
int balota;
int numeroBalota;
int generalDelay = 3000;

void setup() {
  lcd.begin(16, 2);
  pinMode(switchPin, INPUT);
  lcd.print("Numeros del");
  lcd.setCursor(0,1);
  lcd.print("Baloto");
  numeroBalota = 1;

  randomSeed(analogRead(A0));  // Inicializar la semilla del generador de números aleatorios con un valor analógico
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(generalDelay);
  lcd.clear();

  if(numeroBalota < 6) {

    balota = random(1,46);

    lcd.setCursor(0,0);
    char buffer[30];
    sprintf(buffer, "Balota # %d", numeroBalota);
    lcd.print(buffer);
    lcd.setCursor(0,1);
    lcd.print(balota);

    numeroBalota = numeroBalota +1;
  }
  else {
    balota = random(1,17);
    lcd.setCursor(0,0);
    lcd.print("Super Balota");
    lcd.setCursor(0,1);
    lcd.print(balota);

    delay(generalDelay);

    lcd.clear();
    lcd.print("Mucha Suerte");
    exit(0);
  }
}
