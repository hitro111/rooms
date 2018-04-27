#define OUT_AR_OK "1"
#define OUT_AR_NO "0"

#define OUT_SPUTNIK_FINISHED "1"
#define OUT_SPUTNIK_RUNNING "0"

#define OUT_MECHBOX_OPEN "1"
#define OUT_MECHBOX_CLOSED "0"

#define BEEP 3
#define BEEP_DELAY 300
#define BEEP_COUNT 3

#define argonPin 4

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <EEPROM.h>


long totalMilliLitresA;
bool tOpen = false;
bool ok = false;
bool over = false;
bool argonOk = false;
bool sputnikOk = false;
bool mechBoxOpen = false;

void setVolume();
void setTankOpen();

struct TankState {
  bool tOpen;
  long total;
  bool sputnikOk;
  bool mechBoxOpen;
};

byte mac[] = { 0x77, 0xBE, 0xBE, 0x07, 0xFE, 0xEE };
byte ip[] = { 192, 168, 0, 14 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

LiquidCrystal_I2C lcd(0x27, 20, 4);
#define lockPin 9
#define resetPin 7

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
byte letter_B[8] = // Б
{
  B11111, B10001, B10000, B11110, B10001, B10001, B11110, B00000,
};

byte sensorInterrupt = 0;
byte sensorPin = 2;

// The hall-effect flow sensor outputs approximately 4.5 (Closer to 7.5-7.6) pulses per second per litre/minute of flow.
float calibrationFactor = 7.55;

volatile byte pulseCount;

float flowRate;
long flowMilliLitres;

unsigned long oldTime;

#define RES_VAL 123

void loadResult()
{
  byte val1 = EEPROM.read(0);

  if (val1 == RES_VAL)
  {
    TankState ts;
    EEPROM.get(1, ts);
    tOpen = ts.tOpen;
    sputnikOk = ts.sputnikOk;
    mechBoxOpen = ts.mechBoxOpen;

    client.publish("space/console/out", "Sputnik load result: " + sputnikOk);
    client.publish("space/sputnik/out", sputnikOk ? OUT_SPUTNIK_FINISHED : OUT_SPUTNIK_RUNNING);
    client.publish("space/mechbox/out", mechBoxOpen ? OUT_MECHBOX_OPEN : OUT_MECHBOX_CLOSED);

    totalMilliLitresA = ts.total;

    if (tOpen)
    {
      setTankOpen();
      setVolume();
    }
  }
}

void saveState()
{
  TankState tankState = {
    tOpen,
    totalMilliLitresA,
    sputnikOk,
    mechBoxOpen
  };
  EEPROM.write(0, RES_VAL);
  EEPROM.put(1, tankState);
}

void clearResult()
{
  EEPROM.write(0, 0);
}

void __init()
{
  digitalWrite(lockPin, HIGH);
  lcd.clear();
  delay(100);
  lcd.setCursor(0, 0); // Устанавливаем курсор в начало 1 строки
  lcd.print("\6a\2: \4\5\2p\3\1         "); // Бак: закрыт
  lcd.setCursor(0, 1); // Устанавливаем курсор в начало 1 строки
  lcd.print("0 mL            "); // Бак: закрыт
  digitalWrite(sensorPin, HIGH);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitresA = 0;
  oldTime = 0;

  tOpen = false;
  sputnikOk = false;
  mechBoxOpen = false;
}

char buf[5];
void publishVol()
{
  String s = String(totalMilliLitresA);
  s.toCharArray(buf, 5);
  client.publish("space/tank/out", buf);
}

void setup()
{
  pinMode(BEEP, OUTPUT);
  pinMode(argonPin, INPUT);
  digitalWrite(BEEP, HIGH);
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  lcd.init(); // Инициализация lcd
  lcd.backlight(); // Включаем подсветку
  lcd.clear();
  // ткыза
  // 12345
  lcd.createChar(1, letter_t);
  lcd.createChar(2, letter_k);
  lcd.createChar(3, letter_ii);
  lcd.createChar(4, letter_z);
  lcd.createChar(5, letter_a);
  lcd.createChar(6, letter_B);

  pinMode(lockPin, OUTPUT);
  Serial.begin(9600);
  digitalWrite(lockPin, LOW);
  lcd.clear();
  delay(50);
  lcd.setCursor(0, 0); // Устанавливаем курсор в начало 1 строки
  lcd.print("Waiting...");

  pinMode(sensorPin, INPUT);

  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
  connect();
  __init();
  loadResult();
  publishVol();
}

void pulseCounter()
{
  pulseCount++;
}

long long lastSputnikOk = 0;
void loop()
{
  if (!client.connected()) {
    connect();
  }

  client.loop();

  if (digitalRead(argonPin) == HIGH && !argonOk)
  {
    argonOk = true;
    client.publish("space/console/out", "Argon inserted");
    client.publish("space/argonbox/out", OUT_AR_OK);
  }
  else if (digitalRead(argonPin) == LOW && argonOk)
  {
    argonOk = false;
    client.publish("space/console/out", "Argon removed");
    client.publish("space/argonbox/out", OUT_AR_NO);
  }

  if (analogRead(A0) > 500 && analogRead(A1) > 500)
  {
    if (millis() - lastSputnikOk > 500)
    {
      if (lastSputnikOk == 0)
        lastSputnikOk = millis();
      else
      {
        client.publish("space/console/out", "Sputnik signal received");
        client.publish("space/sputnik/out", "1");
        lastSputnikOk = millis();
      }
    }
  }
  else
  {
    lastSputnikOk = 0;
  }

  if (analogRead(A2) > 500 && analogRead(A3) > 500 && !mechBoxOpen)
  {
    client.publish("space/console/out", "Mechbox open");
    client.publish("space/mechbox/out", OUT_MECHBOX_OPEN);
    mechBoxOpen = true;
    saveState();
  }

    if ((millis() - oldTime) > 1000)
    {
      detachInterrupt(sensorInterrupt);

      flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;

      oldTime = millis();

      flowMilliLitres = (flowRate / 60) * 1000;

      totalMilliLitresA += flowMilliLitres;

      if (flowMilliLitres > 0)
      {
        setVolume();

        publishVol();
      }

      saveState();

      pulseCount = 0;

      attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
    }
}

void setVolume()
{
  lcd.setCursor(0, 1);
  lcd.print(totalMilliLitresA);
  lcd.print(" mL");
}

void setTankOpen()
{
  client.publish("space/console/out", "Opening tank...");
  //lcd.clear();
  //delay(50);
  lcd.setCursor(0, 0); // Устанавливаем курсор в начало 1 строки
  lcd.print("\6a\2: o\1\2p\3\1         ");
  digitalWrite(lockPin, LOW);

  for (int i = 0; i < BEEP_COUNT; ++i)
  {
    lcd.noBacklight();
    digitalWrite(BEEP, LOW);
    delay(BEEP_DELAY);
    lcd.backlight();
    digitalWrite(BEEP, HIGH);
    delay(BEEP_DELAY);
  }

}

void setTankClosed()
{
  client.publish("space/console/out", "Closing tank...");
  //lcd.clear();
  //delay(50);
  lcd.setCursor(0, 0); // Устанавливаем курсор в начало 1 строки
  lcd.print("\6a\2: \4\5\2p\3\1         "); // Бак: закрыт
  digitalWrite(lockPin, HIGH);
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  if (payload == "r")
  {
    hard_Reboot();
  }
  if (payload == "p")
  {    
    client.publish("space/ping/out", "tank");
  }
  if (payload == "1")
  {
    tOpen = true;
    setTankOpen();
  }
  if (payload == "0")
  {
    tOpen = false;
    setTankClosed();
  }
  if (payload == "i")
  {
    clearResult();
    __init();
  }
  else if (payload == "a")
  {
    client.publish("space/argonbox/out", argonOk ? OUT_AR_OK : OUT_AR_NO);
    client.publish("space/sputnik/out", sputnikOk ? OUT_SPUTNIK_FINISHED : OUT_SPUTNIK_RUNNING);
    client.publish("space/mechbox/out", mechBoxOpen ? OUT_MECHBOX_OPEN : OUT_MECHBOX_CLOSED);
    publishVol();
  }
  else if (payload == "b") //submit (light were off)
  {
    client.publish("space/console/out", "Received sputnik submission");
    sputnikOk = true;

    saveState();
  }
}

// DEFAULT

void connect() {
  Serial.print("connecting...");
  int n = 0;
  while (!client.connect("stank")) {
    delay(1000);
    n++;
    if (n > 5)
      hard_Reboot();
  }

  Serial.println("\nconnected!");
  client.subscribe("space/reset");
  client.subscribe("space/ping/in");
  client.subscribe("space/tank/reset");
  client.subscribe("space/tank/in");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}






