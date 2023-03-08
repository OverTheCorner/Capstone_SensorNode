#include <Arduino.h>
#include <Adafruit_AS7341.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

#define NUM_OF_DATA_CHANNELS 12
#define PASSWORD "201900044"

// Central Node number: 9212940431

void ds3231Init();
void as7341Init();
void sim800lInit();
void sendTextMsg();
void updateSerial();
void waitForKey();
void updateChannelData();

RTC_DS3231 rtc;
Adafruit_AS7341 as7341;
SoftwareSerial sim800l(10, 11); // RX, TX pins
uint16_t channelData[NUM_OF_DATA_CHANNELS];
String textMessage;
DateTime logDate;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  ds3231Init();

  as7341Init();
  sim800lInit();
  waitForKey();
  updateChannelData();
  waitForKey();
  sendTextMsg();
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

void ds3231Init()
{
  rtc.begin();
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }

  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void as7341Init()
{
  Serial.println("Intitializing AS7341...");
  if (!as7341.begin())
  {
    Serial.println("Could not find AS7341");
    while (1)
    {
      delay(10);
    }
  }

  as7341.setATIME(100);
  as7341.setASTEP(999);
  as7341.setGain(AS7341_GAIN_256X);
  Serial.println("AS7341 Ready to use!\n");
}

void sim800lInit()
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
  Serial.println("SIM800L ready to use!\n");
}

void sendTextMsg()
{
  textMessage = "201900044";
  textMessage += "\n";
  logDate = rtc.now();
  textMessage += String(logDate.day()) + "/" + String(logDate.month()) + "/" + String(logDate.year());
  textMessage += " " + String(logDate.hour()) + ":" + String(logDate.minute()) + "\n";

  for (int i = 0; i < NUM_OF_DATA_CHANNELS; i++)
  {
    textMessage += String(channelData[i]);
    textMessage += ",";
  }
  textMessage += "\n";

  Serial.println("Will now proceed to send a text message...");
  sim800l.println("AT+CMGF=1"); // Set SMS text mode
  delay(1000);
  sim800l.println("AT+CMGS=\"+639274325235\""); // Replace with your phone number
  delay(1000);
  sim800l.println(textMessage); // SMS message
  delay(1000);
  sim800l.println((char)26); // End of message character
  delay(1000);
  Serial.println("Message sent!\n");
}

/**
 * @brief Will wait for keyboard input before proceeding
 *
 * NOTE: character needs can be anything but the ENTER key
 *
 */
void waitForKey()
{
  // prompt user for input
  Serial.read();
  Serial.println("Please enter any character to continue:");
  while (!Serial.available())
  {
    // j
  }
  Serial.println("Key pressed! will now proceed with program\n");
  Serial.read(); // read and discard input
}

void updateChannelData()
{
  Serial.println("Updating Channel Data...");
  if (!as7341.readAllChannels(channelData))
  {
    Serial.println("Error reading all channels!");
    return;
  }
  Serial.println("Channel Data Updated!\n");
}