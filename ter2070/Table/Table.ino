//#define TRACE
//#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <DFPlayer_Mini_Mp3.h>
#include <PCF8574.h>

#include <nfc.h>
#include <Wire.h>

#include "LedControl.h"

#define ACC "ttable"
byte mac[] = { 0x16, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 66 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

PCF8574 eqPcf;
#define PCF_SIZE 8
#define PCF_OFF HIGH
#define PCF_ON LOW

SoftwareSerial mySerial(99, 2);
LedControl lc = LedControl(9, 8, 4, 1);

int soundsMp3 = 13;
int pwrTransferSound = 14;
int timerSoundMp3 = 3; //TODO
int timerFinishedMp3 = 4; //TODO
bool isTimer = false;
bool lightOn = false;
unsigned long long lightTime = 0;
#define LIGHT_ON_TIME 30000

int prevVal = -1;
int bPrevVal = -1;

#define TABLE_LIGHT_BTN A3
#define SOUND_BTN A0
#define SND_CABLE A1

NFC_Module nfc;
u8 buf[32], sta;

#define DEV_ID '7'

#define BLOCKS 2
#define BLOCK1_VAL 550
#define BLOCK2_VAL 820
#define TABLE_VAL 4
#define TABLE_LIGHT_PWR_NEEDED 10
#define SOUND_PWR_NEEDED 5
#define CARD_LIMIT 999
#define SOUND_SECONDS 9

#define BATTERY_LIGHT_PIN 5
#define TABLE_LIGHT_PIN 6

int values[BLOCKS] = {BLOCK1_VAL, BLOCK2_VAL};
int power = TABLE_VAL;

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
  client.publish("ter2070/e/tablePwr", DEV_ID + String(power));
#endif
}
void __init()
{
  // Initialize the MAX7219 device
  lc.shutdown(0, false); // Enable display
  lc.setIntensity(0, 10); // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0); // Clear display register

  values[0] = BLOCK1_VAL;
  values[1] = BLOCK2_VAL;
  power = TABLE_VAL;
  isTimer = false;
  lightOn = false;
  prevVal = bPrevVal = -1;
}

void setup() {
#ifdef TRACE
  Serial.begin(9600);
#endif

  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);

  nfc.begin();
#ifndef NO_SERVER
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
#endif

#ifdef TRACE
  Serial.println("setup started");
#endif

  uint32_t versiondata = nfc.get_version();
  int retries = 5;
  while (retries >= 0)
  {
    versiondata = nfc.get_version();
    if (versiondata) {
      break;
    }
#ifdef TRACE
    Serial.print(F("Didn't find PN53x board"));
#endif
    retries--;

    if (retries == 0)
    {
#ifdef TRACE
      Serial.print(F("Didn't find PN53x board"));
#endif
      digitalWrite(resetPin, HIGH);
    }
  }

  nfc.SAMConfiguration();

  mySerial.begin(9600);
  mp3_set_serial (mySerial);
  mp3_set_volume (80);
  mp3_stop();
  delay (200);

  pinMode(BATTERY_LIGHT_PIN, OUTPUT);
  digitalWrite(BATTERY_LIGHT_PIN, LOW);

  pinMode(TABLE_LIGHT_PIN, OUTPUT);
  digitalWrite(TABLE_LIGHT_PIN, LOW);

  pinMode(TABLE_LIGHT_BTN, INPUT);
  pinMode(SOUND_BTN, INPUT);
  pinMode(SND_CABLE, INPUT);

  eqPcf.begin(0x22);
  for (int i = 0; i < PCF_SIZE; ++i)
  {
    eqPcf.pinMode(i, OUTPUT);
    eqPcf.digitalWrite(i, PCF_OFF);
  }

  __init();


#ifdef TRACE
  Serial.println("setup ok");
#endif
}

bool isBatteryDirty = false;
bool isDisplayDirty = false;
bool isTimerDirty = false;

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

    lc.setDigit(0, 0, n1000, false);
    lc.setDigit(0, 1, n100, false);
    lc.setDigit(0, 2, n10, false);
    lc.setDigit(0, 3, n1, false);
  }
}

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

    lc.setDigit(0, 5, n100, false);
    lc.setDigit(0, 6, n10, false);
    lc.setDigit(0, 7, n1, false);
  }
}

void clearBattery()
{
  if (isBatteryDirty)
  {
    lc.setRow(0, 5, B00000000);
    lc.setRow(0, 6, B00000000);
    lc.setRow(0, 7, B00000000);
    isBatteryDirty = false;
    bPrevVal = -1;
  }
}

void clearDisplay()
{
  if (isBatteryDirty || isDisplayDirty || isTimerDirty)
  {
    lc.clearDisplay(0);
    isBatteryDirty = isDisplayDirty = isTimerDirty = false;
    bPrevVal = prevVal = -1;
  }
}

bool transferPower(int card, bool toCard, int amount)
{
  if (toCard)
  {
    amount = power - amount >= 0 ? amount : power;

    if (values[card] + amount < CARD_LIMIT)
    {
      values[card] += amount;
      power -= amount;
    }

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
  on ? digitalWrite(BATTERY_LIGHT_PIN, HIGH) : digitalWrite(BATTERY_LIGHT_PIN, LOW);
}

bool state = false;
void sound(bool on)
{
  if (on && !state)
  {
#ifdef TRACE
    Serial.println("PLAY TRANSFER");
#endif
    state = true;
    mp3_play (pwrTransferSound);
    delay (50);
    mp3_single_loop (true);
    delay (50);
    mp3_single_loop (true);
  }
  else if (!on && state)
  {
#ifdef TRACE
    Serial.println("STOP TRANSFER");
#endif
    mp3_stop();
    delay(50);
    mp3_stop();
    state = false;
  }
}

#define CD 20000
unsigned long long cd = 0;
int prevSec = -1;
void startCd()
{
  clearDisplay();
  cd = millis() + CD;
}

void timerSound(bool on)
{
  if (on)
  {

#ifdef TRACE
    Serial.println("MP3 TIMER ON");
#endif
    mp3_play (timerSoundMp3);
    delay (50);
    mp3_single_loop (true);
    delay (50);
    mp3_single_loop (true);
  }
  else if (!on)
  {

#ifdef TRACE
    Serial.println("OFF TIMER MP3");
#endif
    mp3_stop();
    delay(50);
    mp3_stop();
  }
}

void timerSoundFinished()
{

#ifdef TRACE
  Serial.println("TIMER MP3 FINISH");
#endif
  mp3_play (timerFinishedMp3);
  delay (10);
}


unsigned long long startBattery = 0;
unsigned long long lastUpdate = 0;
bool powerLeft;
int found = -1;
bool toCard = false;
bool sndCable = false;
void loop() {

#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
#ifdef TRACE
    Serial.println("connected");
#endif
  }
#endif

  if (cd > millis())
  {
    if (!isTimer)
    {
      timerSound(true);
      isTimer = true;
    }

    if (!isTimerDirty)
    {
      lc.setDigit(0, 0, 0, false);
      lc.setDigit(0, 1, 0, true);

      lc.setChar(0, 5, '-', false);
      lc.setChar(0, 6, '-', false);
      lc.setChar(0, 7, '-', false);
    }

    isTimerDirty = true;

    long ms = cd - millis();
    int sec = ms / 1000;
    if (prevSec != sec)
    {
      prevSec = sec;
    }

    lc.setDigit(0, 2, (sec / 10) % 10, false);
    lc.setDigit(0, 3, sec % 10, false);

    if (ms < 100)
    {
      isTimer = false;
      timerSound(false);
      timerSoundFinished();
      cd = 0;
      clearDisplay();
    }

    return;
  }
  else
  {
    setPower();
  }

  if (analogRead(TABLE_LIGHT_BTN) < 100 && power >= TABLE_LIGHT_PWR_NEEDED)
  {
    
#ifdef TRACE
    Serial.println("LIGHT ON");
#endif
    if (!lightOn)
    {
      sound(true);
      for (int i = 0; i < TABLE_LIGHT_PWR_NEEDED; ++i)
      {
        power -= 1;
        setPower();
      }
      sound(false);

      digitalWrite(TABLE_LIGHT_PIN, HIGH);
      lightOn = true;
      lightTime = millis() + LIGHT_ON_TIME;
    }
  }

  if (lightOn && millis() > lightTime)
  {
    lightOn = false;
    digitalWrite(TABLE_LIGHT_PIN, LOW);
  }
  

  if (analogRead(SOUND_BTN) < 100 && power >= SOUND_PWR_NEEDED)
  {
    sound(true);
    for (int i = 0; i < SOUND_PWR_NEEDED; ++i)
    {
      power -= 1;
      setPower();
    }
    sound(false);

    if (analogRead(SND_CABLE) < 100)
    {

#ifdef TRACE
      Serial.println("PLAY SOUNDS");
#endif
      mp3_set_volume(80);
      sndCable = true;
      mp3_play (soundsMp3);
    }

    byte val = 0;
    for (int i = 0; i < SOUND_SECONDS * 10; ++i)
    {
      delay(100);

      if (analogRead(SND_CABLE) < 100 && !sndCable)
      {
        mp3_set_volume(0);
        sndCable = false;
      }
      else if (sndCable)
      {
        mp3_set_volume(80);
        sndCable = true;
      }
      
      client.loop();

      for (int j = 0; j < PCF_SIZE; ++j)
      {
        eqPcf.digitalWrite(j, j >= val ? PCF_ON : PCF_OFF);
      }


      val += 1;
      val = val % PCF_SIZE;
    }

    for (int j = 0; j < PCF_SIZE; ++j)
    {
      eqPcf.digitalWrite(j, PCF_OFF);
    }
    mp3_set_volume(80);

    startCd();
  }

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
        delay(5);
        if (powerLeft)
          light(true);

        powerLeft ? sound(true) : sound(false);
      }
    }
  }
  else
  {
    found = -1;
    light(false);
    sound(false);
    startBattery = 0;
    clearBattery();
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

    if (topic.equals("ter2070/e/tablePwr"))
    {
      //ignore self events
      char dev_id = payload[0];
      if (dev_id == DEV_ID)
        return;

      payload.remove(0, 1);
      power = payload.toInt();
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
  client.subscribe("ter2070/ttable/reset");
  client.subscribe("ter2070/e/block1Pwr");
  client.subscribe("ter2070/e/block2Pwr");
  client.subscribe("ter2070/e/tablePwr");
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
