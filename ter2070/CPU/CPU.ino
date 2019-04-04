//#define TRACE
//#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

#include <nfc.h>
#include <Wire.h>
#include "LedControl.h"

#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
SoftwareSerial mySerial(52, 1);

#define ACC "tcpu"
byte mac[] = { 0x14, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 64 }; // <- change to match your network

#define DEV_ID '5'
LedControl lc = LedControl(9, 8, 6, 1);
#define BATTERY_LIGHT_PIN 5

#define LightGun_neon        4//5
#define LightGun_front_led   3//4
#define LightGun_led_strips  2//3

#define NEON_LIGHT_PIN        11 //neon (was 12)

NFC_Module nfc;
u8 buf[32], sta;

#define CARD1_IN       A0
#define CARD1_LED      13

#define BTN_SHOOT A2
#define PWR_CABLE A3

#define CARD2_IN       A1
#define CARD2_LED      12

EthernetClient net;
MQTTClient client;

#define BLOCKS 2
#define BLOCK1_VAL 180
#define BLOCK2_VAL 112
#define CPU_VAL 0
#define CPU_NEEDED_VAL 100
#define FAIL_SHOT_VAL 10

int values[BLOCKS] = {BLOCK1_VAL, BLOCK2_VAL};
volatile int power = CPU_VAL;

byte bufs[BLOCKS][4] = {
  {134, 1, 219, 31},
  {134, 123, 232, 31}
};

byte decs[BLOCKS][4] = {
  {134, 57, 190, 31},
  {134, 82, 168, 31}
};

bool shotReady = false;
int pwrTransferSound = 14;

void updateCard(int id)
{
#ifndef NO_SERVER
  if (id == 0)
  {
    //client.publish("ter2070/e/block1Pwr", DEV_ID + String(values[id]));
  }
  else
  {
    //client.publish("ter2070/e/block2Pwr", DEV_ID + String(values[id]));
  }
#endif
}

void updatePower()
{
#ifndef NO_SERVER
  //client.publish("ter2070/e/cpuPwr", DEV_ID + String(power));
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
  power = CPU_VAL;

  shotReady = false;
}

bool isCard1(int val)
{
  return val < 800 && val > 300;
}

bool isCard2(int val)
{
  return val < 100;
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

  pinMode(LightGun_neon, OUTPUT);
  pinMode(LightGun_front_led, OUTPUT);
  pinMode(LightGun_led_strips, OUTPUT);
  pinMode(NEON_LIGHT_PIN, OUTPUT);
  pinMode(CARD1_LED, OUTPUT);
  pinMode(CARD2_LED, OUTPUT);

  digitalWrite(LightGun_front_led, LOW);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(CARD1_LED, LOW);
  digitalWrite(CARD2_LED, LOW);

  pinMode(CARD1_IN, INPUT);
  pinMode(CARD2_IN, INPUT);
  pinMode(BTN_SHOOT, INPUT);
  pinMode(PWR_CABLE, INPUT);

  mySerial.begin (9600);
  mp3_set_serial (mySerial);
  mp3_set_volume (30);
  __init();
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

    if (!shotReady && useNeon)
      digitalWrite(NEON_LIGHT_PIN, LOW);
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

void gun_fire_fucked_up() {


  digitalWrite(LightGun_front_led, LOW);
  digitalWrite(LightGun_neon, HIGH);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  analogWrite (LightGun_led_strips, 10);

  delay (100);
  mp3_play (12);


  digitalWrite(LightGun_neon, LOW);
  analogWrite (LightGun_led_strips, 1);
  delay (160);
  digitalWrite(LightGun_neon, HIGH);
  analogWrite (LightGun_led_strips, 80);
  delay (70);
  digitalWrite(LightGun_neon, LOW);
  analogWrite (LightGun_led_strips, 5);
  delay (130);
  digitalWrite(LightGun_neon, HIGH);
  analogWrite (LightGun_led_strips, 10);
  delay (100);
  digitalWrite(LightGun_neon, HIGH);
  analogWrite (LightGun_led_strips, 2);

  delay (700);
  digitalWrite(LightGun_neon, HIGH);
  analogWrite (LightGun_led_strips, 80);
  delay (80);
  digitalWrite(LightGun_neon, LOW);
  analogWrite (LightGun_led_strips, 0);
  delay (80);
  digitalWrite(LightGun_neon, HIGH);
  analogWrite (LightGun_led_strips, 10);
  delay (80);
  digitalWrite(LightGun_neon, LOW);
  analogWrite (LightGun_led_strips, 1);
  delay (80);
  digitalWrite(LightGun_neon, HIGH);
  analogWrite (LightGun_led_strips, 10);
  delay (80);
  digitalWrite(LightGun_neon, LOW);
  analogWrite (LightGun_led_strips, 2);
  delay (50);
  digitalWrite(LightGun_neon, HIGH);
  analogWrite (LightGun_led_strips, 10);
  delay(100);
}

void gun_charging() {

  analogWrite  (LightGun_led_strips, 0);
  digitalWrite(LightGun_front_led, LOW);
  digitalWrite(LightGun_neon, LOW);
  digitalWrite(NEON_LIGHT_PIN, HIGH);

  delay (100);
  mp3_play (11);
  neon_1();
  analogWrite  (LightGun_led_strips, 1);
  neon_1();
  analogWrite  (LightGun_led_strips, 2);
  neon_1();
  analogWrite  (LightGun_led_strips, 3);
  neon_1();
  analogWrite  (LightGun_led_strips, 4);
  neon_1();
  analogWrite  (LightGun_led_strips, 5);
  delay (200);
}

void gun_fire() {
  digitalWrite(LightGun_front_led, LOW);
  digitalWrite(LightGun_neon, HIGH);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  delay (100);
  mp3_play (13);

  for (int i = 10; i < 50; i++)
  {
    analogWrite  (LightGun_led_strips, i);
    delay(30);
  }

  for (int i = 50; i < 255; i++)
  {
    analogWrite  (LightGun_led_strips, i);
    delay(4);
  }
  delay(100);
#ifdef TRACE
  Serial.println("SHOT");
#endif
#ifndef NO_SERVER
  client.publish("ter2070/e/gunshot", "1");
#endif
  front_fire();
  front_fire();
  front_fire();
  front_fire();
  front_fire();
  front_fire();
  front_fire();
  digitalWrite(LightGun_front_led, LOW);
  digitalWrite(LightGun_neon, LOW);

  for (int i = 255; i >= 0; i--)
  {
    analogWrite  (LightGun_led_strips, i);
    delay(3);
  }
}

void neon_1() {
  delay (300);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, HIGH);
  delay (100);
  digitalWrite(NEON_LIGHT_PIN, HIGH);
  digitalWrite(LightGun_neon, LOW);
  delay (50);
  digitalWrite(NEON_LIGHT_PIN, HIGH);
  digitalWrite(LightGun_neon, HIGH);
  delay (100);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, LOW);
  delay (50);
  digitalWrite(NEON_LIGHT_PIN, HIGH);
  digitalWrite(LightGun_neon, HIGH);
  delay (150);
  delay (100);
  digitalWrite(NEON_LIGHT_PIN, HIGH);
  digitalWrite(LightGun_neon, LOW);
}


void neon_2() {

  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, HIGH);
  delay (90);
  digitalWrite(NEON_LIGHT_PIN, HIGH);
  digitalWrite(LightGun_neon, LOW);
  delay (30);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, HIGH);
  delay (90);
  digitalWrite(NEON_LIGHT_PIN, HIGH);
  digitalWrite(LightGun_neon, LOW);
  delay (30);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, HIGH);
  delay (120);
  digitalWrite(NEON_LIGHT_PIN, HIGH);
  digitalWrite(LightGun_neon, LOW);
  delay (30);
}

void neon_3() {

  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, HIGH);
  delay (60);
  digitalWrite(NEON_LIGHT_PIN, HIGH);
  digitalWrite(LightGun_neon, LOW);
  delay (15);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, HIGH);
  delay (60);
  digitalWrite(NEON_LIGHT_PIN, HIGH);
  digitalWrite(LightGun_neon, LOW);
  delay (15);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, HIGH);
  delay (80);
  digitalWrite(NEON_LIGHT_PIN, HIGH);
  digitalWrite(LightGun_neon, LOW);
  delay (10);
}

void neon_4() {
  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, HIGH);
  delay (60);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, LOW);
  delay (5);
  digitalWrite(NEON_LIGHT_PIN, HIGH);
  digitalWrite(LightGun_neon, HIGH);
  delay (80);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, LOW);
  delay (5);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, HIGH);
  delay (80);
  digitalWrite(NEON_LIGHT_PIN, LOW);
  digitalWrite(LightGun_neon, LOW);
  delay (0);
}


void front_fire() {

  digitalWrite(LightGun_front_led, HIGH);
  delay (20);
  digitalWrite(LightGun_front_led, LOW);
  delay (20);

  digitalWrite(LightGun_front_led, HIGH);
  delay (20);
  digitalWrite(LightGun_front_led, LOW);
  delay (20);

  digitalWrite(LightGun_front_led, HIGH);
  delay (20);
  digitalWrite(LightGun_front_led, LOW);
  delay (20);

  digitalWrite(LightGun_front_led, HIGH);
  delay (20);
  digitalWrite(LightGun_front_led, LOW);
  delay (20);

  digitalWrite(LightGun_front_led, HIGH);
  delay (20);
  digitalWrite(LightGun_front_led, LOW);
  delay (20);
}

unsigned long lastMillis = 0;
unsigned long long startBattery = 0;
unsigned long long lastUpdate = 0;
bool powerLeft;
int found = -1;
bool toCard = false;
void loop() {
  setPower();

#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();

    Serial.println("Connected");
  }
#endif

  if (analogRead(PWR_CABLE) < 500 && power >= CPU_NEEDED_VAL)
  {
    sound(true);
    light(false);
    bool neonOn = false;
    for (int i = 0; i < CPU_NEEDED_VAL; ++i)
    {
      power -= 1;
      setPower();
      delay(10);
      neonOn = !neonOn;
      digitalWrite(NEON_LIGHT_PIN, neonOn);
    }
    digitalWrite(NEON_LIGHT_PIN, LOW);
    sound(false);

    shotReady = true;
    digitalWrite(NEON_LIGHT_PIN, HIGH);
  }

  if (shotReady && analogRead(BTN_SHOOT) < 500)
  {
#ifdef TRACE
    Serial.println("READY");
#endif
    shotReady = false;
    gun_charging();

    if (isCard1(analogRead(CARD1_IN)) && isCard2(analogRead(CARD2_IN)))
    {
      gun_fire();
    }
    else
    {
      gun_fire_fucked_up();

      sound(true);
      for (int i = 0; i < CPU_NEEDED_VAL - FAIL_SHOT_VAL; ++i)
      {
        power += 1;
        setPower();
        delay(10);
      }
      sound(false);
      digitalWrite(NEON_LIGHT_PIN, LOW);
    }
  }

  digitalWrite(CARD1_LED, isCard1(analogRead(CARD1_IN)));
  digitalWrite(CARD2_LED, isCard2(analogRead(CARD2_IN)));

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

    if (topic.equals("ter2070/e/cpuPwr"))
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
  client.subscribe("ter2070/e/cpuPwr");

  client.subscribe("ter2070/reset");
  client.subscribe("ter2070/ping/in");
  client.subscribe("ter2070/tcpu/reset");
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
