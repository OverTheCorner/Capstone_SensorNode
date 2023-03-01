#include <Arduino.h>
#include <Adafruit_AS7341.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

void gsmInit();
void sendTextMsg();
void updateSerial();

SoftwareSerial sim800l(10, 11); // RX, TX pins

void setup()
{
  Serial.begin(9600);
  gsmInit();
}

void loop()
{
  // Empty loop
}

void updateSerial()
{
  delay(500);
  while (Serial.available())
  {
    sim800l.write(Serial.read()); // Forward what Serial received to Software Serial Port
  }
  while (sim800l.available())
  {
    Serial.write(sim800l.read()); // Forward what Software Serial received to Serial Port
  }
}

void gsmInit()
{
  sim800l.begin(9600);
  delay(1000);
  Serial.println("Initializing SIM800L...");
  delay(3000);
  sim800l.println("AT"); // Once the handshake test is successful, it will back to OK
  updateSerial();
  sim800l.println("AT+CSQ"); // Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  sim800l.println("AT+CCID"); // Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  sim800l.println("AT+CREG?"); // Check whether it has registered in the network
  updateSerial();
}

void sendTextMsg()
{
  sim800l.println("AT+CMGF=1"); // Set SMS text mode
  delay(1000);
  sim800l.println("AT+CMGS=\"+639274325235\""); // Replace with your phone number
  delay(1000);
  sim800l.println("Hello from Arduino!"); // SMS message
  delay(1000);
  sim800l.println((char)26); // End of message character
  delay(1000);

  Serial.println("Message sent!");
}