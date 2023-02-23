#include <Arduino.h>
#include <Adafruit_AS7341.h>

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
}

void loop()
{
  digitalWrite(LED_BUILTIN, 0);
  Serial.println("LED OFF");
  delay(1000);
  digitalWrite(LED_BUILTIN, 1);
  Serial.println("LED ON");
  delay(1000);
}