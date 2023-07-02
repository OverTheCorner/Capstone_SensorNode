#include <Arduino.h>
#include <Adafruit_AS7341.h>
#include <RTClib.h>
#include <SoftwareSerial.h>
#include <LowPower.h>

#define NUM_OF_DATA_CHANNELS 12
#define PASSWORD "201900044"
#define PUMP_PIN 8
#define VALVE_PIN 7
#define LED_PIN 6

// Central Node number: 9212940431

void relayInit();
void lightInit();
void ds3231Init();
void as7341Init();
void sim800lInit();
void turnOnLight();
void turnOffLight();
void turnOnPump();
void turnOffPump();
void openValve();
void closeValve();
void sendTextMsg();
void sendTextMsgDiag();
void updateSerial();
void waitForKey();
void updateChannelData();
void printChannelData();
void sim800lSleep();
void sim800lWake();
void arduinoSleep8s();
void arduinoSleep();
void wakeUp();

RTC_DS3231 rtc;
Adafruit_AS7341 as7341;
SoftwareSerial sim800l(10, 11); // RX, TX pins
uint16_t channelData[NUM_OF_DATA_CHANNELS];
uint32_t channelDataTrans[NUM_OF_DATA_CHANNELS];
const uint32_t channelDataCalibration[12] = {6166, 42578, 18477, 36295, 65535, 4543, 41208, 31943, 22575, 9921, 65535, 4543};
String textMessage;
DateTime logDate;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  relayInit();
  lightInit();
  ds3231Init();
  as7341Init();
  sim800lInit();
  // waitForKey();
  // updateChannelData();
  // waitForKey();
  // sendTextMsg();
}

void loop()
{
  // Collect Sample
  closeValve();
  delay(1000);
  turnOnPump();
  delay(3000);
  turnOffPump();

  // Scan Sample
  turnOnLight();
  delay(1000);
  updateChannelData();
  delay(1000);
  turnOffLight();

  // Send Text Message
  printChannelData();
  sim800lInit();
  sendTextMsg();
  delay(5000);
  sendTextMsgDiag();

  // release sample
  openValve();

  // Enter Sleep Mode
  turnOffLight();
  turnOffPump();
  sim800lSleep();
  delay(1000);
  arduinoSleep();

  // Wake Up
  Serial.println("Arduino awake!");
  sim800lWake();
  delay(1000);
}

void relayInit()
{
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH);
  digitalWrite(VALVE_PIN, HIGH);
}

void lightInit()
{
  pinMode(LED_PIN, OUTPUT);
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

void turnOnLight()
{
  digitalWrite(LED_PIN, HIGH);
}

void turnOffLight()
{
  digitalWrite(LED_PIN, LOW);
}

void turnOnPump()
{
  Serial.println("PUMP ON...");
  digitalWrite(PUMP_PIN, LOW);
}

void turnOffPump()
{
  Serial.println("PUMP OFF...");
  digitalWrite(PUMP_PIN, HIGH);
}

void openValve()
{
  Serial.println("VALVE OPEN...");
  digitalWrite(VALVE_PIN, HIGH);
}

void closeValve()
{
  Serial.println("VALVE CLOSE...");
  digitalWrite(VALVE_PIN, LOW);
}

void sendTextMsg()
{
  textMessage = "201900046";
  textMessage += "\n";
  logDate = rtc.now();
  textMessage += String(logDate.day()) + "/" + String(logDate.month()) + "/" + String(logDate.year());
  textMessage += " " + String(logDate.hour()) + ":" + String(logDate.minute()) + "\n";

  for (int i = 0; i < NUM_OF_DATA_CHANNELS; i++)
  {
    textMessage += String(channelDataTrans[i]);
    textMessage += ",";
  }
  textMessage += "\n";

  Serial.println("Will now proceed to send a text message...");
  sim800l.println("AT+CMGF=1"); // Set SMS text mode
  delay(1000);
  sim800l.println("AT+CMGS=\"+639194253489\""); // Replace with your phone number
  delay(1000);
  sim800l.println(textMessage); // SMS message
  delay(1000);
  sim800l.println((char)26); // End of message character
  delay(1000);
  Serial.println("Message sent!\n");
}

void sendTextMsgDiag()
{
  Serial.println("Sending text to Diagnostic center...");
  textMessage = "201900046";
  textMessage += "\n";
  logDate = rtc.now();
  textMessage += String(logDate.day()) + "/" + String(logDate.month()) + "/" + String(logDate.year());
  textMessage += " " + String(logDate.hour()) + ":" + String(logDate.minute()) + "\n";

  for (int i = 0; i < NUM_OF_DATA_CHANNELS; i++)
  {
    textMessage += String(channelDataTrans[i]);
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
  for (int i = 0; i < NUM_OF_DATA_CHANNELS; i++)
  {
    channelDataTrans[i] = channelData[i];
    channelDataTrans[i] = (channelDataTrans[i] * 100) / channelDataCalibration[i];
  }
  Serial.println("Channel Data Updated!\n");
}

void printChannelData()
{
  Serial.println("Channel Data Trans:");
  for (int i = 0; i < NUM_OF_DATA_CHANNELS; i++)
  {
    Serial.println(channelDataTrans[i]);
  }
  Serial.println("End of Channel Data Trans");
}

void sim800lSleep()
{
  Serial.println("SIM800L going to sleep...");
  sim800l.println("AT+CSCLK=2");
  updateSerial();
  Serial.println("SIM800L in sleep mode!");
}

void sim800lWake()
{
  Serial.println("SIM800L waking up...");
  sim800l.println("AT");
  delay(100);
  sim800l.println("AT+CSCLK=0");
  updateSerial();
  delay(100);
  sim800l.println("AT+CSCLK?");
  updateSerial();
  Serial.println("SIM800L is awake!");
}

void arduinoSleep8s()
{
  Serial.println("Arduino going to sleep...");
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

void arduinoSleep()
{
  Serial.println("Arduino going to sleep...");
  delay(1000);
  // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(digitalPinToInterrupt(2), wakeUp, LOW);
  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(2));
}

void wakeUp()
{
  Serial.println("Arduino waking up...");
}