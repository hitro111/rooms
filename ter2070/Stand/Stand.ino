#define TRACE
//#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <nfc.h>
#include <Wire.h>
#include <Servo.h>

#include "LedControl.h"
EthernetClient net;
#define DEV_ID '6'
byte mac[] = { 0x15, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 65 }; // <- change to match your network

LedControl lc = LedControl(9, 8, A0, 1);
#define SOUND_PIN 2
#define LIGHT_PIN 4
#define BTN_GUN A3
#define BTN_HAND A2

#define PWR_CABLE A1  //low = connected

#define ACC "tstand"
MQTTClient client;

NFC_Module nfc;
u8 buf[32], sta;

#define BLOCKS 2
#define BLOCK1_VAL 550
#define BLOCK2_VAL 820
#define STAND_VAL 1
#define HAND_PWR 10
#define GUN_PWR 20
#define CARD_LIMIT 999

#define GUN_PIN 5
#define HAND_LED 3


Servo hand_servo; // объявляем переменную servo типа "hand_servo"

int values[BLOCKS] = {BLOCK1_VAL, BLOCK2_VAL};
int power = STAND_VAL;

int prevVal = -1;
int bPrevVal = -1;

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
  client.publish("ter2070/e/standPwr", DEV_ID + String(power));
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
  power = STAND_VAL;
  
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
  pinMode(SOUND_PIN, OUTPUT);
  pinMode(BTN_GUN, INPUT);
  pinMode(BTN_HAND, INPUT);
  pinMode(HAND_LED, OUTPUT);
  pinMode(GUN_PIN, OUTPUT);

  pinMode(GUN_PIN, LOW);
  digitalWrite(HAND_LED, LOW);
  digitalWrite(SOUND_PIN, LOW);
  digitalWrite (LIGHT_PIN, LOW);

  hand_servo.attach(6);
  hand_servo.write(0);

  __init();
#ifdef TRACE
  Serial.print("ok1234");
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
  on ? digitalWrite(LIGHT_PIN, HIGH) : digitalWrite(LIGHT_PIN, LOW);
}

void sound(bool on)
{
  on ? digitalWrite(SOUND_PIN, HIGH) : digitalWrite(SOUND_PIN, LOW);
}

void moveHand()
{
  for (int i = 0; i < 70; i++)
  {
    analogWrite(HAND_LED, i);
    delay(25);
  }

#ifndef NO_SERVER
  client.loop();
#endif

  analogWrite(HAND_LED, 120);
  hand_servo.write(120);
  delay(1000);
#ifndef NO_SERVER
  client.loop();
#endif
  delay(750);

#ifndef NO_SERVER
  client.loop();
#endif
  for (int i = 120; i >= 80; i--)
  {
    hand_servo.write(i);
    analogWrite(HAND_LED, i);
    delay(25);
  }
#ifndef NO_SERVER
  client.loop();
#endif

  for (int i = 80; i >= 0; i--)
  {
    hand_servo.write(i);
    analogWrite(HAND_LED, i);
    delay(15);
  }
}

void runGun()
{
  analogWrite(GUN_PIN, 255);
  delay(1500);
  analogWrite(GUN_PIN, 0);
  delay(200);
}

#define CD 60000
unsigned long long cd = 0;
int prevSec = -1;
void startCd()
{
  clearDisplay();
  cd = millis() + CD;
}

unsigned long long startBattery = 0;
unsigned long long lastUpdate = 0;
bool powerLeft;
int found = -1;
bool toCard = false;
void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
    Serial.println(F("connected"));
  }
#endif

  if (digitalRead(PWR_CABLE)) //disconnected
  {
    clearDisplay();
    return;
  }
  else if (cd > millis())
  {
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
      digitalWrite(SOUND_PIN, HIGH);
      delay(1);
      digitalWrite(SOUND_PIN, LOW);
      prevSec = sec;
    }

    lc.setDigit(0, 2, (sec / 10) % 10, false);
    lc.setDigit(0, 3, sec % 10, false);

    if (ms < 100)
    {
      cd = 0;
      clearDisplay();
      digitalWrite(SOUND_PIN, HIGH);
      delay(50);
      digitalWrite(SOUND_PIN, LOW);
      delay(50);
      digitalWrite(SOUND_PIN, HIGH);
      delay(100);
      digitalWrite(SOUND_PIN, LOW);
    }

    return;
  }
  else
  {
    setPower();
  }

  if (analogRead(BTN_HAND) < 500 && power >= HAND_PWR)
  {
    digitalWrite(SOUND_PIN, LOW);
    digitalWrite (LIGHT_PIN, LOW);
    for (int i = 0; i < HAND_PWR; ++i)
    {
      power -= 1;
      setPower();
      sound(true);
      delay(10);
      sound(false);
    }

    moveHand();
    startCd();
  }


  if (analogRead(BTN_GUN) < 500 && power >= GUN_PWR)
  {
    digitalWrite(SOUND_PIN, LOW);
    digitalWrite (LIGHT_PIN, LOW);
    for (int i = 0; i < GUN_PWR; ++i)
    {
      power -= 1;
      setPower();
      sound(true);
      delay(10);
      sound(false);
    }

    runGun();
    startCd();
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
    digitalWrite(LIGHT_PIN, LOW);
    digitalWrite(SOUND_PIN, LOW);
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

    if (topic.equals("ter2070/e/standPwr"))
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
  client.subscribe("ter2070/tstand/reset");
  client.subscribe("ter2070/e/block1Pwr");
  client.subscribe("ter2070/e/block2Pwr");
  client.subscribe("ter2070/e/standPwr");
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
