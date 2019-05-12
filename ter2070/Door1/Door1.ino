//#define TRACE
//#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <nfc.h>
#include <Wire.h>

#include "LedControl.h"
EthernetClient net;
#define DEV_ID '1'
byte mac[] = { 0x07, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 57 }; // <- change to match your network

LedControl lc = LedControl(9, 8, 6, 1);
#define DOOR_LOCK 3 //when LOW -> Open
#define KEY_LOCK 4 //when LO W -> drop
#define SOUND_PIN 2
#define BTN_IN A0 //BTN when low
#define FUSE_IN 1 //<500 => ON
#define LIGHT_PIN 5

#define ACC "tdoor1"
MQTTClient client;

NFC_Module nfc;
u8 buf[32], sta;

//const size_t bufferSize = JSON_OBJECT_SIZE(2);
//StaticJsonDocument<bufferSize> jsonBuffer;
//JsonObject root = jsonBuffer.to<JsonObject>();

#define BLOCKS 2
#define BLOCK1_VAL 650
#define BLOCK2_VAL 820
#define DOOR1_VAL 17
#define PWR_NEEDED 200
#define CARD_LIMIT 999

int values[BLOCKS] = {BLOCK1_VAL, BLOCK2_VAL};
int power = DOOR1_VAL;

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
  client.publish("ter2070/e/door1Pwr", DEV_ID + String(power));
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
  power = DOOR1_VAL;
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

  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(KEY_LOCK, OUTPUT);
  pinMode(DOOR_LOCK, OUTPUT);
  pinMode(SOUND_PIN, OUTPUT);
  pinMode(BTN_IN, INPUT);

  digitalWrite(SOUND_PIN, LOW);
  digitalWrite (LIGHT_PIN, LOW);
  digitalWrite (KEY_LOCK, LOW);
  digitalWrite (DOOR_LOCK, HIGH);

  __init();
#ifdef TRACE
  Serial.print("ok1234");
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
    power += amount;
    values[card] -= amount;

    return values[card] != 0;
  }
}

void light(bool on)
{
  on ? digitalWrite(LIGHT_PIN, HIGH) : digitalWrite(LIGHT_PIN, LOW);
}

void sound(bool on)
{
  on ? digitalWrite(SOUND_PIN, HIGH) : digitalWrite(SOUND_PIN, LOW);
}


unsigned long long startBattery = 0;
unsigned long long lastUpdate = 0;
bool transferActive;
int found = -1;
bool toCard = false;
bool fuseOff;

void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
    Serial.println(F("connected"));
  }
#endif
  if (analogRead(FUSE_IN) < 500) //если есть пружинка
  {
    fuseOff = false;
    setPower();

    if (analogRead(BTN_IN) < 500 && power >= PWR_NEEDED)
    {
      for (int i = 0; i < PWR_NEEDED; ++i)
      {
        power -= 1;
        setPower();
        sound(true);
        delay(10);
        sound(false);
      }

      digitalWrite(KEY_LOCK, HIGH);
      delay(500);
      digitalWrite(KEY_LOCK, LOW);
    }

    sta = nfc.InListPassiveTarget(buf);

    if (sta && buf[0] == 4)
    {
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
          if (transferActive)
            sound(true);
          delay(5);
          sound(false);
          if (transferActive)
            light(true);
        }
      }
    }
    else
    {
      found = -1;
      digitalWrite(LIGHT_PIN, LOW);
      digitalWrite(SOUND_PIN, LOW);
      startBattery = 0;
      clearBattery();
    }
  }
  else  if (!fuseOff)
  {
    clearDisplay();
    fuseOff = true;
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

    if (topic.equals("ter2070/tdoor1/device"))
    {
      if (payload.equals("o"))
      {
        digitalWrite(DOOR_LOCK, LOW);
      }
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

    if (topic.equals("ter2070/e/door1Pwr"))
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
    {
      Serial.println(F("hard reboot"));
      hard_Reboot();
    }
  }

  client.subscribe("ter2070/reset");
  client.subscribe("ter2070/ping/in");
  client.subscribe("ter2070/tdoor1/reset");
  client.subscribe("ter2070/tdoor1/device");
  client.subscribe("ter2070/e/block1Pwr");
  client.subscribe("ter2070/e/block2Pwr");
  client.subscribe("ter2070/e/door1Pwr");
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
