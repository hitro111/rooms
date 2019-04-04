//#define TRACE
//#define NO_SERVER
#define resetPin 7
//#define DIAGNOSTICS
//#define NO_NFC

#if defined DIAGNOSTICS && !defined NO_SERVER
#define NO_SERVER
#endif

#if defined DIAGNOSTICS && !defined TRACE
#define TRACE

#endif

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

#include <nfc.h>
#include <Wire.h>
#include "LedControl.h"

#define ACC "ttv"
#define DEV_ID '4'

#define SOUND_PIN 2
#define BATTERY_LIGHT_PIN 3
#define BTN_IN A1 //BTN when low
#define TV_PIN 5
#define CARD_PIN A0
#define CARD_LED A3

#define ON_DELAY 200
#define ENDVID_DELAY 200
#define EXIT_DELAY 5000
int video_delay[] = {400, 600, 800, 1000, 1200};

byte mac[] = { 0x13, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 63 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

LedControl lc = LedControl(9, 8, 6, 1);
NFC_Module nfc;
u8 buf[32], sta;

#define BLOCKS 2
#define BLOCK1_VAL 180
#define BLOCK2_VAL 112
#define TV_VAL 77
#define TV_POWER_NEEDED 50

int values[BLOCKS] = {BLOCK1_VAL, BLOCK2_VAL};
int power = TV_VAL;

#define NOCARD_VAL 1023
#define CARD1_VAL 198
#define CARD2_VAL 448
#define CARD3_VAL 596
#define CARD4_VAL 708
#define CARD5_VAL 923

#define CARD_EPS 20

#define NO_CMD '0'
#define ON_CMD 'o'
#define ENDVID_CMD 'e'
#define EXIT_CMD 'x'
#define VID1_CMD '1'
#define VID2_CMD '2'
#define VID3_CMD '3'
#define VID4_CMD '4'
#define VID5_CMD '5'

char lastSent = NO_CMD;
bool tvOn = false;

byte bufs[BLOCKS][4] = {
  {134, 1, 219, 31},
  {134, 123, 232, 31}
};

byte decs[BLOCKS][4] = {
  {134, 57, 190, 31},
  {134, 82, 168, 31}
};

void updateCard(int id)
{
#ifndef NO_SERVER
  if (id == 0)
  {
    client.publish("ter2070/e/block1Pwr", DEV_ID + String(values[id]));
  }
  else
  {
    client.publish("ter2070/e/block2Pwr", DEV_ID + String(values[id]));
  }
#endif
}

void updatePower()
{
#ifndef NO_SERVER
  client.publish("ter2070/e/tvPwr", DEV_ID + String(power));
#endif
}

void __init()
{
#ifdef TRACE
  Serial.println(F("init"));
#endif
  // Initialize the MAX7219 device
  lc.shutdown(0, false); // Enable display
  lc.setIntensity(0, 10); // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0); // Clear display register

  values[0] = BLOCK1_VAL;
  values[1] = BLOCK2_VAL;
  power = TV_VAL;
  tvOn = false;
  lastSent = NO_CMD;
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

#ifndef NO_NFC
  uint32_t versiondata = nfc.get_version();
  int retries = 5;
  while (retries >= 0)
  {
    versiondata = nfc.get_version();
    if (versiondata) {
      break;
    }
#ifdef TRACE
    Serial.println(F("Didn't find PN53x board..."));
#endif
    retries--;
    delay(500);

    if (retries == 0)
    {
#ifdef TRACE
      Serial.println(F("Reboot (NFC)"));
#endif
      digitalWrite(resetPin, HIGH);
    }
  }

  nfc.SAMConfiguration();
#endif

  pinMode(SOUND_PIN, OUTPUT);
  pinMode(BATTERY_LIGHT_PIN, OUTPUT);
  pinMode(TV_PIN, OUTPUT);
  pinMode(BTN_IN, INPUT);
  pinMode(CARD_PIN, INPUT);
  pinMode(CARD_LED, OUTPUT);

  digitalWrite (SOUND_PIN, LOW); //inverted
  digitalWrite (TV_PIN, LOW); //inverted
  digitalWrite (BATTERY_LIGHT_PIN, LOW);
  digitalWrite (CARD_LED, LOW);

  __init();  

#ifdef TRACE
  Serial.println(F("setup OK"));
#endif
}

bool isBatteryDirty = false;
bool isDisplayDirty = false;

int prevVal = -1;
void setPower()
{
  int p = power;
  if (prevVal != power)
  {
    prevVal = p;
    updatePower();
    isDisplayDirty = true;
    int n1 = p % 10;
    p /= 10;
    int n10 = p % 10;
    p /= 10;
    int n100 = p % 10;
    p /= 10;
    int n1000 = p % 10;

    Serial.println("setBattery");
    lc.setDigit(0, 0, n1000, false);
    lc.setDigit(0, 1, n100, false);
    lc.setDigit(0, 2, n10, false);
    lc.setDigit(0, 3, n1, false);
  }
}

int bPrevVal = -1;
void setBattery(int card)
{
  int p = values[card];
  if (bPrevVal != p)
  {
    bPrevVal = p;
    updateCard(card);
    isBatteryDirty = true;
    int n1 = p % 10;
    p /= 10;
    int n10 = p % 10;
    p /= 10;
    int n100 = p % 10;

    Serial.println("setBattery");
    lc.setDigit(0, 5, n100, false);
    lc.setDigit(0, 6, n10, false);
    lc.setDigit(0, 7, n1, false);
  }
}

void clearBattery()
{
  if (isBatteryDirty)
  {
    Serial.println("clearBattery");
    lc.setRow(0, 5, B00000000);
    lc.setRow(0, 6, B00000000);
    lc.setRow(0, 7, B00000000);
    isBatteryDirty = false;
    bPrevVal = -1;
  }
}

void clearDisplay()
{
  if (isBatteryDirty || isDisplayDirty)
  {
    Serial.println("clearDisplay");
    lc.clearDisplay(0);
    isBatteryDirty = isDisplayDirty = false;
    bPrevVal = prevVal = -1;
  }
}

bool transferPower(int card, bool toCard, int amount)
{
  if (toCard)
  {
    amount = power - amount >= 0 ? amount : power;
    power -= amount;
    values[card] += amount;
    return power != 0;
  }
  else
  {
    amount = values[card] - amount >= 0 ? amount : values[card];
    power += amount;

    values[card] -= amount;
    return values[card] != 0;
  }
}

void light(bool on)
{
  if (on)
  {
    digitalWrite(BATTERY_LIGHT_PIN, HIGH);
  }
  else
  {
    digitalWrite(BATTERY_LIGHT_PIN, LOW);
  }
}

void sound(bool on)
{
  on ? digitalWrite(SOUND_PIN, HIGH) : digitalWrite(SOUND_PIN, LOW);
}

void CMD_ON()
{
  if (lastSent == ON_CMD)
    return;

#ifdef TRACE
  Serial.println("TV ON");
#endif

  lastSent = ON_CMD;
  digitalWrite(TV_PIN, HIGH);
  delay(ON_DELAY);
  digitalWrite(TV_PIN, LOW);
}

void CMD_VID(int i)
{
  if (lastSent == i + '1' || lastSent == NO_CMD)
    return;


#ifdef TRACE
  Serial.print("VID: ");
  Serial.println(i);
#endif

  lastSent = i + '1';
  digitalWrite(TV_PIN, HIGH);
  delay(video_delay[i]);
  digitalWrite(TV_PIN, LOW);
}

void CMD_ENDVID()
{
  if (lastSent == ENDVID_CMD || lastSent == NO_CMD || lastSent == ON_CMD)
  {
    return;
  }

#ifdef TRACE
  Serial.println("ENDVID");
#endif

  lastSent = ENDVID_CMD;
  digitalWrite(TV_PIN, HIGH);
  delay(ENDVID_DELAY);
  digitalWrite(TV_PIN, LOW);
}

void CMD_EXIT()
{
#ifdef TRACE
  Serial.println("EXIT");
#endif
  lastSent = NO_CMD;
  digitalWrite(TV_PIN, HIGH);
  delay(EXIT_DELAY);
  digitalWrite(TV_PIN, LOW);
}

#ifdef DIAGNOSTICS
char c;
void handleDiagnostics()
{
  if (Serial.available())
  {
    while (Serial.available())
    {
      c = Serial.read();
    }

    switch (c)
    {
      case ON_CMD:
        CMD_ON();
        break;
      case VID1_CMD:
        Serial.println("Video 1");
        CMD_VID(0);
        break;
      case VID2_CMD:
        Serial.println("Video 2");
        CMD_VID(1);
        break;
      case VID3_CMD:
        Serial.println("Video 3");
        CMD_VID(2);
        break;
      case VID4_CMD:
        Serial.println("Video 4");
        CMD_VID(3);
        break;
      case VID5_CMD:
        Serial.println("Video 5");
        CMD_VID(4);
        break;
      case ENDVID_CMD:
        Serial.println("finish");
        CMD_ENDVID();
        break;
      case EXIT_CMD:
        Serial.println("Exit");
        CMD_EXIT();
        break;
    }
  }
}
#endif

unsigned long lastMillis = 0;
unsigned long long startBattery = 0;
unsigned long long lastUpdate = 0;
bool powerLeft;
int found = -1;
bool toCard = false;
void loop() {
#ifndef NO_NFC
  setPower();
#endif

#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
#ifdef TRACE
    Serial.println(F("connected"));
#endif
  }
#endif

#ifdef DIAGNOSTICS
  handleDiagnostics();
  return;
#endif


  if (!tvOn)
  {
    if (analogRead(BTN_IN) < 500 && power >= TV_POWER_NEEDED)
    {
      for (int i = 0; i < TV_POWER_NEEDED; ++i)
      {
        power -= 1;
        setPower();
        sound(true);
        delay(10);
        sound(false);
      }

      CMD_ON();

      tvOn = true;
    }
  }
  else
  {
    int v = analogRead(CARD_PIN);

#ifdef TRACE
    Serial.println(v);
#endif
   
    if (v > NOCARD_VAL - CARD_EPS)
    {
      digitalWrite(CARD_LED, LOW);
      CMD_ENDVID();
    }
    if (v > CARD1_VAL - CARD_EPS && v < CARD1_VAL + CARD_EPS)
    {
      digitalWrite(CARD_LED, HIGH);
      CMD_VID(0);
    }
    else if (v > CARD2_VAL - CARD_EPS && v < CARD2_VAL + CARD_EPS)
    {
      digitalWrite(CARD_LED, HIGH);
      CMD_VID(1);
    }
    else if (v > CARD3_VAL - CARD_EPS && v < CARD3_VAL + CARD_EPS)
    {
      digitalWrite(CARD_LED, HIGH);
      CMD_VID(2);
    }
    else if (v > CARD4_VAL - CARD_EPS && v < CARD4_VAL + CARD_EPS)
    {
      digitalWrite(CARD_LED, HIGH);
      CMD_VID(3);
    }
    else if (v > CARD5_VAL - CARD_EPS && v < CARD5_VAL + CARD_EPS)
    {
      digitalWrite(CARD_LED, HIGH);
      CMD_VID(4);
    }//...
  }

#ifndef NO_NFC
  sta = nfc.InListPassiveTarget(buf);
  if (sta && buf[0] == 4)
  {
    for (int i = 0; i < BLOCKS; ++i)
    {
      if (arr_equal(bufs[i], buf + 1, 4))
      {
        found = i;
        toCard = true;
      }
      else if (arr_equal(decs[i], buf + 1, 4))
      {
        found = i;
        toCard = false;
      }
    }

    if (found >= 0)
    {
      setPower();
      setBattery(found);

      if (startBattery == 0)
      {
        startBattery = millis();
      }

      if (millis() - startBattery > 2000)
      {
        if (millis() - lastUpdate > 50)
        {
          lastUpdate = millis();

          transferPower(found, toCard, 1);

          setPower();
          setBattery(found);
        }

        powerLeft = toCard ? power : values[found];
        light(false);
        if (powerLeft)
          sound(true);
        delay(5);
        sound(false);
        if (powerLeft)
          light(true);
      }
    }
  }
  else
  {
    found = -1;
    light(false);
    digitalWrite(SOUND_PIN, LOW);
    startBattery = 0;
    clearBattery();
  }
#endif
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  if (topic.startsWith("ter"))
  {
    if (topic.equals("ter2070/e/block1Pwr"))
    {
      //ignore self events
      char dev_id = payload[0];
      if (dev_id == DEV_ID)
        return;

      payload.remove(0, 1);
      values[0] = payload.toInt();
    }

    if (topic.equals("ter2070/e/block2Pwr"))
    {
      //ignore self events
      char dev_id = payload[0];
      if (dev_id == DEV_ID)
        return;

      payload.remove(0, 1);
      values[1] = payload.toInt();
    }

    if (topic.equals("ter2070/e/tvPwr"))
    {
      //ignore self events
      char dev_id = payload[0];
      if (dev_id == DEV_ID)
        return;
      payload.remove(0, 1);
      power = payload.toInt();
    }

    if (topic.equals("ter2070/c/tvOff"))
    {
      CMD_EXIT();
    }

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
  client.subscribe("ter2070/ttv/reset");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}

bool arr_equal(byte const * x, byte const * y, size_t n)
{
  for (size_t i = 0; i < n; i++)
    if (x[i] != y[i])
      return false;
  return true;
}
