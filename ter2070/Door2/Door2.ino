/*
  SUBSCRIBE:
  "ter2070/tdoor2/device"
  "a" = alarm
  "n" = no alarm
  "o" = open manually

  PUBLISH:
  "ter2070/tdoor2/server"
  "1" - door opened
*/

//#define TRACE
//#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define ACC "tdoor2"
#define DOOR_LOCK 3
#define RESET_PIN 7
#define KEY_PIN A1

#define SMOKE_PIN A0
#define ON LOW
#define OFF HIGH

LiquidCrystal_I2C lcd(0x27, 20, 4);
byte letter_t[8] = // т
{
  B00000, B00000, B11111, B00100, B00100, B00100, B00100, B00000,
};
byte letter_k[8] = // к
{
  B00000, B00000, B10001, B10010, B11100, B10010, B10001, B00000,
};
byte letter_ii[8] = // ы
{
  B00000, B00000, B10001, B10001, B11101, B10101, B11101, B00000,
};
byte letter_z[8] = // з
{
  B00000, B00000, B11110, B00001, B00110, B00001, B11110, B00000,
};
byte letter_a[8] = // а
{
  B00000, B00000, B01110, B00001, B01111, B10001, B01111, B00000,
};

byte mac[] = { 0x06, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 56 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

bool smokeOn = false;
unsigned long long smokeOnTime = 0;
#define SMOKE_TIME 20000

enum State
{
  Init,
  Waiting,
  Alarm,
  Opening,
  Open
};

State state;
bool isAlarm = false;

void __init()
{
  state = Waiting;
  isAlarm = false;

  lcd.clear();
  lcd.setCursor(0, 0);                 // Устанавливаем курсор в начало 1 строки
  lcd.print("O\1ce\2: \4\5\2p\3\1");   // Отсек: закрыт
  digitalWrite (DOOR_LOCK, HIGH);
}

void setup() {
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);

#ifndef NO_SERVER
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
#endif

#ifdef TRACE
  Serial.begin(9600);
#endif

  lcd.init();                       // Инициализация lcd
  lcd.backlight();                  // Включаем подсветку
  // ткыза
  // 12345
  lcd.createChar(1, letter_t);
  lcd.createChar(2, letter_k);
  lcd.createChar(3, letter_ii);
  lcd.createChar(4, letter_z);
  lcd.createChar(5, letter_a);
  lcd.setCursor(0, 0);
  pinMode(DOOR_LOCK, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  pinMode(SMOKE_PIN, OUTPUT);

  digitalWrite(DOOR_LOCK, HIGH);
  digitalWrite(RESET_PIN, LOW);
  digitalWrite(SMOKE_PIN, OFF);

  __init();
}

int val;

void handleSmoke()
{
  if (smokeOn && smokeOnTime == 0) //need to turn on smoke
  {
    digitalWrite(SMOKE_PIN, ON);
    smokeOnTime = millis();
  }

  if (smokeOn && millis() - smokeOnTime > SMOKE_TIME)
  {
    digitalWrite(SMOKE_PIN, OFF);
    smokeOn = smokeOnTime = 0;
  }
}

void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif

  handleSmoke();

  switch (state)
  {
    case Waiting:
      val = analogRead(KEY_PIN);

      if (val < 500)
      {
        if (!isAlarm)
        {
          state = Opening;
        }
        else
        {
          lcd.noBacklight();
          delay(300);
          lcd.backlight();
          delay(300);
          lcd.noBacklight();
          delay(300);
          lcd.backlight();
          delay(300);
        }
      }
      break;
    case Opening:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("O\1ce\2: o\1\2p\3\1");
      client.publish("ter2070/tdoor2/server", "1");
      digitalWrite (DOOR_LOCK, LOW);
      state = Open;
      break;
    case Open:
      break;
    case Alarm:
      isAlarm = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ALARM!");
      state = Waiting;
      break;
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

    if (topic == "ter2070/tdoor2/device")
    {
      if (payload == "a") //alarm on
      {
        state = Alarm;
      }
      else if (payload == "n") //alarm off
      {
        isAlarm = false;
        lcd.clear();
        lcd.setCursor(0, 0);                 // Устанавливаем курсор в начало 1 строки
        lcd.print("O\1ce\2: \4\5\2p\3\1");   // Отсек: закрыт
      }
      else if (payload == "o") //open manually
      {
        isAlarm = false;
        state = Opening;
      }
    }

    if (topic == "ter2070/c/smoke")
    {
      if (payload == "1")
      {
        smokeOn = true;
      }
      else if (payload == "0")
      {
        digitalWrite(SMOKE_PIN, OFF);
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

  client.subscribe("ter2070/reset");
  client.subscribe("ter2070/ping/in");
  client.subscribe("ter2070/tdoor2/reset");
  client.subscribe("ter2070/tdoor2/device");
  client.subscribe("ter2070/c/smoke");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}
