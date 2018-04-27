#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <Servo.h>

#define MAP_OK_CMD 9
#define RESET_MAP 8

#define resetPin 3
byte mac[] = { 0x10, 0xAD, 0xBE, 0x10, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 18 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

Servo arServo;
Servo n2Servo;
#define AR_SERVO 8
#define N2_SERVO 9

int RedBtnLed = 2;
int RedBtn = 4;
int AirBtn = 5;
int OkislBtn = 6; //Окислитель N204

LiquidCrystal_I2C lcd(0x27, 20, 4);    // Задаем адрес и размерность дисплея.

byte letter_J[8] = // Ж
{
  B10101, B10101, B10101, B01110, B10101, B10101, B10101, B00000,
};
byte letter_D[8] = // Д
{
  B11111, B00101, B00101, B01001, B10001, B11111, B10001, B00000,
};
byte letter_v[8] = // в
{
  B00000, B00000, B11100, B10010, B11100, B10010, B11100, B00000,
};
byte letter_ii[8] = // ы
{
  B00000, B00000, B10001, B10001, B11101, B10101, B11101, B00000,
};
byte letter_k[8] = // к
{
  B00000, B00000, B10001, B10010, B11100, B10010, B10001, B00000,
};
byte letter_l[8] = // л
{
  B00000, B00000, B11111, B00101, B00101, B10101, B01001, B00000,
};

byte all[8] = // Зарисованый прямоугольник
{
  B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111,
};

byte low[8] = // Зарисованый прямоугольник
{
  B00001, B11101, B00001, B00001, B00001, B00001, B00001, B00001,
};

enum Response
{
  None,
  Waiting,
  Ok,
  Error
};

bool flightMap = false;
bool air = false;
bool tankOpen = false;
bool success = false;
int tankVol;
bool argonBoxOk;
Response resp;

void __init()
{
  lcd.clear();
  delay(50);
  lcd.setCursor(0, 0);
  lcd.print("\1P\2-3B3:");
  lcd.setCursor(9, 0);
  lcd.print("\3\4\5\6");  // Выкл
  argonBoxOk = true;
  flightMap = success = tankOpen = air = false;
  resp = None;
  digitalWrite(RedBtnLed, LOW);
  arServo.write(42);
  Serial.print('1');
}

void setup()
{
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  client.begin("192.168.0.91", net);

  pinMode(RedBtnLed, OUTPUT);
  pinMode(RedBtn, INPUT);
  pinMode(AirBtn, INPUT);
  pinMode(OkislBtn, INPUT);

  arServo.attach(AR_SERVO);
  n2Servo.attach(N2_SERVO);

  lcd.init();                          // Инициализация lcd
  lcd.backlight();                     // Включаем подсветку
  // ткыза
  // 12345
  lcd.createChar(1, letter_J);
  lcd.createChar(2, letter_D);
  lcd.createChar(3, letter_v);
  lcd.createChar(4, letter_ii);
  lcd.createChar(5, letter_k);
  lcd.createChar(6, letter_l);
  lcd.createChar(7, all);
  lcd.createChar(0, low);

  lcd.clear();

  delay(100);
  lcd.setCursor(0, 0);
  lcd.print("Waiting...");
  connect();
  __init();
  delay(100);
}

unsigned long long lastErr = 0;

void loop()
{

  client.loop();

  if (!client.connected()) {
    connect();
  }

  if (success)
    return;

  if (Serial.available()) {
    int inByte = Serial.read();

    if (inByte == '1')
    {
      client.publish("space/console/out", "MAP SUCCESS"); // LOG
      flightMap = true;
    }
  }

  int arServoVal = argonBoxOk ? 42 : 122;
  arServo.write(arServoVal);
  int tv = tankVol > 1200 ? 1200 : tankVol;
  int n2ServoVal = map(tv, 0, 1200, 132, 45);
  n2Servo.write(n2ServoVal);

  switch (resp)
  {
    case Waiting:
      return;
    case Ok:
      success = true;
      return;
    case Error:
      success = false;
      if (millis() - lastErr <= 500)
      {
        resp = None;
      }
      break;
  }

  // Значения с потенциометров переводятся в значения
  //Окистлитель
  int val1 = analogRead(0);
  val1 = map(val1, 0, 1023, 100, 0);
  lcd.setCursor(0, 2);
  lcd.print("N204:");
  lcd.setCursor(6, 2);
  lcd.print(val1);
  lcd.print(" ");
  lcd.setCursor(11, 2);
  lcd.print("%");

  //Топливо
  int val2 = analogRead(1);
  val2 = map(val2, 0, 1023, 100, 0);
  lcd.setCursor(0, 3);
  lcd.print("N2H4:");
  lcd.setCursor(6, 3);
  lcd.print(val2);
  lcd.print(" ");
  lcd.setCursor(11, 3);
  lcd.print("%");

  // Нажатие большой красной кнопки
  int sensorValue = digitalRead(RedBtn);
  if (sensorValue == 1)
  {

    if (flightMap && argonBoxOk && air && (val1 >= 10 && val1 <= 15) && val2 == 100 && resp != Error)
    {
      client.publish("space/console/out", "Engine start attempt"); // LOG
      // Выполняем действие
      digitalWrite(RedBtnLed, HIGH);
      client.publish("space/cpupanel/out", "2");
      lcd.setCursor(9, 0);
      lcd.print("\3\5\6 ");  // Вкл

      resp = Waiting;
      return;
    }
    else
    {
      if (millis() - lastErr > 500)
      {
        client.publish("space/console/out", "Engine cannot be started"); // LOG
        delay(20);
        client.publish("space/console/out", "Air open: " + (air ? String("true") : String("false"))); // LOG
        delay(20);
        client.publish("space/console/out", "Map: " + (flightMap ? String("true") : String("false"))); // LOG
        delay(20);
        client.publish("space/console/out", "N204(needed = 10-15%):  " + String(val1) ); // LOG
        delay(20);
        client.publish("space/console/out", "N2H4(needed = 100%): " + String(val2) ); // LOG
        delay(20);
        client.publish("space/console/out", "Argon : " + (argonBoxOk ? String("true") : String("false"))); // LOG
        delay(20);

        client.publish("space/cpupanel/out", "3");


        lcd.setCursor(9, 0);
        lcd.print("\3\5\6 ");  // Вкл
        digitalWrite(RedBtnLed, HIGH);
        delay(600);
        digitalWrite(RedBtnLed, LOW);
        delay(300);
        digitalWrite(RedBtnLed, HIGH);
        delay(300);
        digitalWrite(RedBtnLed, LOW);
        delay(150);
        digitalWrite(RedBtnLed, HIGH);
        delay(150);
        digitalWrite(RedBtnLed, LOW);
        delay(75);
        digitalWrite(RedBtnLed, HIGH);
        delay(75);
        digitalWrite(RedBtnLed, LOW);
        lcd.setCursor(9, 0);
        lcd.print("\3\4\5\6");

        lastErr = millis();
        resp = None;
      }
    }
  }
  else
  {
    // Выполняем действие
    digitalWrite(RedBtnLed, LOW);
    lcd.setCursor(9, 0);
    lcd.print("\3\4\5\6");
  }

  // Тумблер воздух
  int sensorValue1 = digitalRead(AirBtn);
  if (sensorValue1 == 1)
  {
    air = true;
  }
  else
  {
    air = false;
  }


  // Тумблер окислитель
  int sensorValue2 = digitalRead(OkislBtn);
  if (sensorValue2 == 1)
  {
    if (!tankOpen)
    {
      delay(100);
      if (digitalRead(OkislBtn) == 1)
      {
        tankOpen = true;
        client.publish("space/cpupanel/out", "1");
      }
    }
  }
  else
  {
    if (tankOpen)
    {
      tankOpen = false;
      client.publish("space/cpupanel/out", "0");
    }
  }

}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  if (topic == "space/argonbox/out")
  {
    if (payload == "1")
      argonBoxOk = true;
    else if (payload == "0")
      argonBoxOk = false;
  }
  else if (topic == "space/tank/out")
  {
    tankVol = payload.toInt();
  }
  else
  {
    if (payload == "r")
    {
      hard_Reboot();
    }
    else if (payload == "i")
    {
      __init();
    }
    else if (payload == "z")
    {
      resp = Ok;
    }
    else if (payload == "e")
    {
      resp = Error;
    }
    else if (payload = "h")
    {
      flightMap = false;
      Serial.print('9');
    }
    else if (payload == "p")
    {
      client.publish("space/ping/out", "cpupanel");
    }
  }
}

// DEFAULT

void connect() {
  int n = 0;
  while (!client.connect("scpupan")) {
    delay(1000);
    n++;
    if (n > 5)
      hard_Reboot();
  }

  client.subscribe("space/reset");
  client.subscribe("space/cpupanel/in");
  client.subscribe("space/argonbox/out");
  client.subscribe("space/tank/out");
  client.subscribe("space/ping/in");
  client.subscribe("space/cpupanel/reset");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}




