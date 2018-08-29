#define TRACE
#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <nfc.h>
#include <Wire.h>

#include "LedControl.h"

LedControl lc = LedControl(9, 8, 4, 1);
#define DOOR_LOCK 5 //when LOW -> Open
#define KEY_LOCK 6 //when LOW -> drop
#define REBOOT_PIN 7
#define BTN_IN 1 //BTN when low
#define FUSE_IN 3 //<500 => ON

#define ACC "tdoor1"
byte mac[] = { 0x07, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 57 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

NFC_Module nfc;
u8 buf[32], sta;
u8 hextab_my[17] = "0123456789ABCDEF";

int power = 37;
int battery = 163;

void __init()
{
  // Initialize the MAX7219 device
  lc.shutdown(0, false); // Enable display
  lc.setIntensity(0, 10); // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0); // Clear display register
}

void setup() {
  nfc.begin();
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);

#ifndef NO_SERVER
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
#endif

#ifdef TRACE
  Serial.begin(9600);
#endif

  uint32_t versiondata = nfc.get_version();
  if (! versiondata) {
#ifdef TRACE
    Serial.print("Didn't find PN53x board");
#endif
    while (1); // halt
  }

  nfc.SAMConfiguration();
  Serial.print("OK");
  pinMode(DOOR_LOCK, OUTPUT);
  pinMode(KEY_LOCK, OUTPUT);
  pinMode(REBOOT_PIN, OUTPUT);
  digitalWrite (DOOR_LOCK, HIGH);
  digitalWrite (KEY_LOCK, HIGH);
  digitalWrite (REBOOT_PIN, LOW);

  __init();
}

void setPower(int p)
{
  int n1 = p % 10;
  p /= 10;
  int n10 = p % 10;
  p /= 10;
  int n100 = p % 10;
  p /= 10;
  int n1000 = p % 10;

  lc.setDigit(0, 0, n1000, false);
  lc.setDigit(0, 1, n100, false);
  lc.setDigit(0, 2, n10, false);
  lc.setDigit(0, 3, n1, false);
}

void setBattery(int p)
{
  int n1 = p % 10;
  p /= 10;
  int n10 = p % 10;
  p /= 10;
  int n100 = p % 10;

  lc.setDigit(0, 5, n100, false);
  lc.setDigit(0, 6, n10, false);
  lc.setDigit(0, 7, n1, false);
}

void clearBattery()
{
lc.setRow(0,5,B00000000); 

lc.setRow(0,6,B00000000);

lc.setRow(0,7,B00000000);
}

void transferPower(bool toBattery, int amount)
{
  if (toBattery)
  {
    amount = power - amount >= 0 ? amount : power;
    power -= amount;
    battery += amount;
  }
  else
  {
    amount = battery - amount >= 0 ? amount : battery;
    power += amount;
    battery -= amount;
  }
}

unsigned long long startBattery = 0;
unsigned long long lastUpdate = 0;
void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif

  if (analogRead(FUSE_IN) < 500) //если есть пружинка
  {
    setPower(power);

    sta = nfc.InListPassiveTarget(buf);

    if (sta && buf[0] == 4)
    {
      setPower(power);
      setBattery(battery);
      /** the card may be Mifare Classic card, try to read the block */
      //nfc.puthex(buf + 1, buf[0]);

      u32 i;
      //134 123 232 31 - наружу
      //134 82 168 31 - внутрь
      for (i = 0; i < buf[0]; i++)
      {
        //Serial.print(buf[i + 1]);
        //Serial.write(' ');
      }
      

      if (buf[2] == 123)
      {
        if (startBattery == 0)
        {
          startBattery = millis();
        }

        if (millis() - startBattery > 2000)
        {
          if (millis() - lastUpdate > 50)
          {
            lastUpdate = millis();
            transferPower(false, 1);
          }
        }
      }
      else if (buf[2] == 82)
      {
        if (startBattery == 0)
        {
          startBattery = millis();
        }

        if (millis() - startBattery > 2000)
        {
          if (millis() - lastUpdate > 50)
          {
            lastUpdate = millis();
            transferPower(true, 1);
          }
        }
      }
    }
    else
    {
      startBattery = 0;
      clearBattery();
    }
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  if (topic.startsWith("ter"))
  {
    if (payload == "r")
    {
      hard_Reboot();
    }
    else if (payload == "i")
    {
      __init();
    }
    else if (payload == "p")
    {
      client.publish("ter2070/ping/out", ACC);
    }
  }
}

// DEFAULT

void connect() {
  int n = 0;
  while (!client.connect(ACC)) {
    delay(1000);
    n++;
    if (n > 5)
      hard_Reboot();
  }

  client.subscribe("ter2070/reset");
  client.subscribe("ter2070/ping/in");
  client.subscribe("ter2070/tdoor1/reset");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}
