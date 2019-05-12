//#define TRACE
//#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

#include "LedControl.h"
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
#include <nfc.h>

#define ACC "tsound"
#define DEV_ID '3'

byte mac[] = { 0x12, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 62 }; // <- change to match your network
EthernetClient net;
MQTTClient client;


#define BATTERY_LIGHT_PIN 2

NFC_Module nfc;
u8 buf[32], sta;

#define BLOCKS 2
#define BLOCK1_VAL 650
#define BLOCK2_VAL 820
#define SND_INIT_VAL 21
#define SND_RES_VAL 1200
#define CARD_LIMIT 999

int values[BLOCKS] = {BLOCK1_VAL, BLOCK2_VAL};
int power = SND_INIT_VAL;

byte bufs[BLOCKS][4] = {
  {134, 1, 219, 31},
  {134, 123, 232, 31}
};

byte decs[BLOCKS][4] = {
  {134, 57, 190, 31},
  {134, 82, 168, 31}
};

SoftwareSerial mySerial(99, 9);
LedControl lc = LedControl(5, 6, 8, 1);


int fullMp3 = 13; // "0013.mp3"
int expected_vals[6] = {LOW, HIGH, HIGH, HIGH, LOW, HIGH};
int needed_sounds[6] = {1, 2, 3, 4, 5, 6}; //"0001.mp3", "0002.mp3"...
int bad_sounds[6] = {7, 8, 9, 10, 11, 12}; //"0007.mp3"...
int pwrTransferSound = 14;
int pins[6] = {3, 4, A3, A2, A1, A0};
int vals_prev[6];
int vals[6];
bool finished = false;

int prevVal = -1;
int bPrevVal = -1;

void updateCard(int card_id)
{
#ifndef NO_SERVER
  if (card_id == 0)
  {
    client.publish("ter2070/e/block1Pwr", DEV_ID + String(values[card_id]));
  }
  else
  {
    client.publish("ter2070/e/block2Pwr", DEV_ID + String(values[card_id]));
  }
#endif
}

void updatePower()
{
#ifndef NO_SERVER
  client.publish("ter2070/e/soundPwr", DEV_ID + String(power));
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
  power = SND_INIT_VAL;
  finished = false;
  
  prevVal = bPrevVal = -1;
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

  pinMode(BATTERY_LIGHT_PIN, OUTPUT);
  digitalWrite (BATTERY_LIGHT_PIN, LOW);

  for (int i = 0; i < 6; ++i)
  {
    pinMode(pins[i], INPUT);
  }
  mySerial.begin(9600);

  // Initialize the MAX7219 device
  lc.shutdown(0, false); // Enable display
  lc.setIntensity(0, 10); // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0); // Clear display register

  mySerial.begin(9600);
  mp3_set_serial (mySerial);
  mp3_set_volume (40);
  delay (200);

  mp3_stop();
  //mp3_play(fullMp3);

  for (int i = 0; i < 6; ++i)
  {
    vals[i] = digitalRead(pins[i]);
  }

  Serial.begin(9600);

  __init();
#ifdef TRACE
  Serial.println(F("setup OK"));
#endif
}

bool isBatteryDirty = false;
bool isDisplayDirty = false;

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
  if (isBatteryDirty || isDisplayDirty)
  {
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

    if (values[card] + amount <= CARD_LIMIT)
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
  if (on)
  {
    digitalWrite(BATTERY_LIGHT_PIN, HIGH);
  }
  else
  {
    digitalWrite(BATTERY_LIGHT_PIN, LOW);
  }
}


bool state = false;
void sound(bool on)
{
  if (on && !state)
  {
    state = true;
    mp3_play (pwrTransferSound);
    delay (50);
    mp3_single_loop (true);
    delay (50);
    mp3_single_loop (true);
  }
  else if (!on && state)
  {
    mp3_stop();
    delay(50);
    mp3_stop();
    state = false;
  }
}

void btnSound(bool needed, int i)
{
  if (!state)
  {
    mp3_play(needed ? needed_sounds[i] : bad_sounds[i]);
  }
}


unsigned long lastMillis = 0;
unsigned long long startBattery = 0;
unsigned long long lastUpdate = 0;
bool transferActive;
int found = -1;
bool toCard = false;
void loop() {

  setPower();

#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif

  bool allCorrect = true;
  for (int i = 0; i < 6; ++i)
  {
    int val = digitalRead(pins[i]);

    if (val != vals[i])
    {
      bool isCorrect = expected_vals[i] == val;
      delay(100);

      btnSound(isCorrect, i);

      vals[i] = val;
    }

    if (vals[i] != expected_vals[i])
      allCorrect = false;
  }

  if (allCorrect && !finished)
  {
    delay(500);
    sound(true);
    for (int i = 0; i < SND_RES_VAL; ++i)
    {
      power += 1;
      setPower();
      delay(10);
    }
    sound(false);
    finished = true;
  }

  sta = nfc.InListPassiveTarget(buf);

  if (sta && buf[0] == 4)
  {
    //nfc.puthex(buf + 1, buf[0]);

    u32 i;
    //134 123 232 31 - наружу
    //134 82 168 31 - внутрь
    for (i = 0; i < buf[0]; i++)
    {
      //Serial.print(buf[i + 1]);
      //Serial.write(' ');
    }

    //Serial.println();

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

        transferActive = toCard ? power > 0 && values[found] < CARD_LIMIT : values[found];


        light(false);
        delay(5);
        if (transferActive)
          light(true);

        transferActive ? sound(true) : sound(false);
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

    if (topic.equals("ter2070/e/soundPwr"))
    {
      //ignore self events
      char dev_id = payload[0];
      if (dev_id == DEV_ID)
        return;

      payload.remove(0, 1);
      power = payload.toInt();
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

  client.subscribe("ter2070/e/block1Pwr");
  client.subscribe("ter2070/e/block2Pwr");
  client.subscribe("ter2070/e/sndPwr");

  client.subscribe("ter2070/reset");
  client.subscribe("ter2070/ping/in");
  client.subscribe("ter2070/tsound/reset");
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

//30 -> 50 -> 60 -> 35 -> 25 -> 15
//50 -> 10 -> 30 -> 15 -> 60 -> 45
