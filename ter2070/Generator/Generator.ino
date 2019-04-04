#define TRACE
#define PCF
//#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#ifdef PCF
#include "PCF8574.h"
#endif
#include <nfc.h>
#include <Wire.h>
#include "LedControl.h"

#define DEV_ID '2'
LedControl lc = LedControl(9, 8, 6, 1);
#define BATTERY_LIGHT_PIN 3 //light block
#define SOUND_PIN 4
#define NEON_LIGHT_PIN 5 //neon light
#define HALL_INTERRUPT 0

NFC_Module nfc;
u8 buf[32], sta;

#ifdef PCF
PCF8574 pcf1;
PCF8574 pcf2;
#endif

#define BLOCKS 2
#define BLOCK1_VAL 180
#define BLOCK2_VAL 112
#define GEN_VAL 66
#define POWER_LIMIT 999
#define CARD_LIMIT 999

int values[BLOCKS] = {BLOCK1_VAL, BLOCK2_VAL};
volatile int power = GEN_VAL;
volatile int powerFract = 0;

byte bufs[BLOCKS][4] = {
  {134, 1, 219, 31},
  {134, 123, 232, 31}
};

byte decs[BLOCKS][4] = {
  {134, 57, 190, 31},
  {134, 82, 168, 31}
};

#define ACC "tgen"
byte mac[] = { 0x11, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 61 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

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
  client.publish("ter2070/e/genPwr", DEV_ID + String(power));
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
  power = GEN_VAL;
}

bool state[16] = {0};
int mappedPos[16] = {15, 0, 12, 5, 4, 7, 9, 1, 3, 8, 2, 11, 13, 14, 6, 10 };
bool inverseVal[16] = {0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0};
bool tumblersDone = false;

bool allOk()
{
  for (int i = 0; i < 16; ++i)
  {
    if (state[i]) return false;
  }
  return true;
}

#ifdef PCF
void pcfWrite(int i, bool val)
{
  if (i < 8)
  {
    pcf1.digitalWrite(i, val ? HIGH : LOW);
  }
  else
  {
    pcf2.digitalWrite(i % 8, val ? HIGH : LOW);
  }
}
#endif

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

#ifdef PCF
  pcf1.begin(0x25);
  pcf2.begin(0x26);
#endif

  pinMode(BATTERY_LIGHT_PIN, OUTPUT);
  pinMode(SOUND_PIN, OUTPUT);
  pinMode(NEON_LIGHT_PIN, OUTPUT);
  attachInterrupt(HALL_INTERRUPT, generate, FALLING);

  digitalWrite (BATTERY_LIGHT_PIN, LOW);
  digitalWrite (SOUND_PIN, LOW);
  digitalWrite (NEON_LIGHT_PIN, LOW);

#ifdef PCF
  for (int i = 0; i < 8; ++i)
  {
    pcf1.pinMode(i, OUTPUT);
    pcf2.pinMode(i, OUTPUT);
  }
#endif

  __init();
#ifdef TRACE
  Serial.println(F("setup OK"));
#endif
}

bool incPowerFract()
{
  powerFract++;
  if (powerFract > 9)
  {
    powerFract = 0;
    return true;
  }

  return false;
}

volatile int hallTicks = 0;
#define TICKS_NEEDED 2
void generate()
{
  if (hallTicks < TICKS_NEEDED)
  {
    hallTicks++;
  }
  else
  {
    hallTicks = 0;
    if (incPowerFract())
    {
      setPower(power + 1);
    }
  }
}

bool setPower(int val)
{
  if (val <= POWER_LIMIT)
  {
    power = val;
    return true;
  }

  return false;
}

bool isBatteryDirty = false;
bool isDisplayDirty = false;

int prevVal = -1;
int prevFractVal = -1;
void setPowerDisplay()
{
  int p = power;
  if (prevVal != power || prevFractVal != powerFract)
  {
    prevVal = p;
    prevFractVal = powerFract;
    updatePower();
    isDisplayDirty = true;
    int n1 = p % 10;
    p /= 10;
    int n10 = p % 10;
    p /= 10;
    int n100 = p % 10;
    lc.setDigit(0, 0, n100, false);
    lc.setDigit(0, 1, n10, false);
    lc.setDigit(0, 2, n1, true);
    lc.setDigit(0, 3, powerFract, false);
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

    if (setPower(power + amount))
    {
      values[card] -= amount;
    }

    return values[card] != 0;
  }
}

byte neonCt = 0;
#define NEON_USE 3
void light(bool on)
{
  neonCt++;
  bool useNeon = (neonCt % NEON_USE) == 0;
  if (on)
  {
    digitalWrite(BATTERY_LIGHT_PIN, HIGH);
    if (useNeon)
      digitalWrite(NEON_LIGHT_PIN, HIGH);
  }
  else
  {
    digitalWrite(BATTERY_LIGHT_PIN, LOW);
    if (useNeon)
      digitalWrite(NEON_LIGHT_PIN, LOW);
  }
}

void sound(bool on)
{
  on ? digitalWrite(SOUND_PIN, HIGH) : digitalWrite(SOUND_PIN, LOW); //inverted
}

unsigned long lastMillis = 0;
unsigned long long startBattery = 0;
unsigned long long lastUpdate = 0;
bool powerLeft;
int found = -1;
bool toCard = false;
int hallPrevVal = HIGH;
void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
#ifdef TRACE
    Serial.println(F("connected"));
#endif
  }
#endif

#ifndef NO_SERVER
  if (!tumblersDone)
  {
    clearDisplay();
    return;
  }
#endif

  setPowerDisplay();
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
      setPowerDisplay();
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

          setPowerDisplay();
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

    if (topic.equals("ter2070/e/genPwr"))
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

    if (topic.equals("ter2070/ttumblr/server"))
    {
#ifdef TRACE
      Serial.println(payload);
#endif
      long tumblers = payload.toInt();

      for (int i = 0; i < 16; ++i)
      {
#ifdef PCF
        bool val = tumblers & (1L << i);
        if (inverseVal[i])
          val = !val;
        pcfWrite(mappedPos[i], val);
#endif
        state[mappedPos[i]] = val;

        Serial.print(state[i]);
        Serial.print(", ");
      }
      Serial.println();

      tumblersDone = allOk();
      if (tumblersDone)
      {
#ifdef TRACE
        Serial.println(F("ALL OK"));
#endif
      }
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
  client.subscribe("ter2070/e/genPwr");

  client.subscribe("ter2070/ping/in");
  client.subscribe("ter2070/tgen/reset");
  client.subscribe("ter2070/ttumblr/server");
  client.subscribe("ter2070/reset");

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
