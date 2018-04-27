#define OUT_FUSES_OK "1"
#define OUT_FUSES_FAIL "0"
#define OUT_FUSES_OK_SILENT "9"
#define OUT_FUSES_FAIL_SILENT "8"

#define OUT_DOOR_OPEN "1"
#define OUT_DOOR_CLOSED "0"
#define OUT_DOOR_OPEN_SILENT "9"
#define OUT_DOOR_CLOSED_SILENT "8"

#define OUT_GAG_REQUEST "3"

#define IN_GAG_OPEN "4"
#define IN_GAG_CLOSED "5"


#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

byte mac[] = { 0x33, 0xAD, 0xBE, 0x3, 0x03, 0x03 };
byte ip[] = { 192, 168, 0, 10 }; // <- change to match your network
EthernetClient net;
MQTTClient client;

#define Pin_Reader_RFID         0
#define Pin_Reader_Fuses        1
#define LED_Strip_White         6
#define LED_Strip_Red           5
#define Electromagnetic_lock    7

LiquidCrystal_I2C lcd(0x27, 20, 4);    // Задаем адрес и размерность дисплея.

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

unsigned long lastMillis = 0;
bool isManual = false;
bool isRedOn = false;

bool fusesNotSet = true;
bool fusesOk = false;
bool isDoorOpen = false;
bool wasDoorOpen = false;
bool gagNotSet = true;
unsigned long long gagLastCall = 0;
bool gagOpen = false;
void initGag();

#define DOOR_OPEN_VAL 123
#define DOOR_CL_VAL 211

bool loadResult()
{
  byte val1 = EEPROM.read(0);
  byte val2 = EEPROM.read(1);

  wasDoorOpen = val1 == DOOR_OPEN_VAL && val2 == DOOR_OPEN_VAL;

  return (val1 == DOOR_OPEN_VAL && val2 == DOOR_OPEN_VAL) || (val1 == DOOR_CL_VAL && val2 == DOOR_CL_VAL);
}

void saveDoorOpen()
{
  EEPROM.write(0, DOOR_OPEN_VAL);
  EEPROM.write(1, DOOR_OPEN_VAL);
}

void clearResult()
{
  EEPROM.write(0, 0);
  EEPROM.write(1, 0);
}


void __init()
{
  lcd.clear();
  delay(50);
  lcd.setCursor(0, 0);                 // Устанавливаем курсор в начало 1 строки
  lcd.print("O\1ce\2: \4\5\2p\3\1");   // Отсек: закрыт
  digitalWrite(Electromagnetic_lock, HIGH);
  isManual = false;

  fusesNotSet = true;
  fusesOk = false;
  isDoorOpen = false;
  gagNotSet = true;
  gagOpen = false;
  gagLastCall = 0;
}

void setup()
{
  pinMode(LED_Strip_White, OUTPUT);
  pinMode(LED_Strip_Red, OUTPUT);
  pinMode(Electromagnetic_lock, OUTPUT);

  lcd.init();                          // Инициализация lcd
  lcd.backlight();                     // Включаем подсветку
  // 12345
  lcd.createChar(1, letter_t);
  lcd.createChar(2, letter_k);
  lcd.createChar(3, letter_ii);
  lcd.createChar(4, letter_z);
  lcd.createChar(5, letter_a);
  lcd.clear();
  delay(50);
  lcd.setCursor(0, 0);                 // Устанавливаем курсор в начало 1 строки
  lcd.print("Waiting...");   // Отсек: закрыт
  digitalWrite(Electromagnetic_lock, HIGH);

  Serial.begin(9600);

  digitalWrite(LED_Strip_White, HIGH);
  digitalWrite(LED_Strip_Red, LOW);
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
  connect();
  lcd.noBacklight();
  delay(100);
  __init();

  loadResult();

  digitalWrite(LED_Strip_Red, HIGH);
  digitalWrite(LED_Strip_White, LOW);
}

bool doorReady()
{
  return (gagOpen || gagNotSet) && fusesOk;
}

unsigned long long fusesFailStart = 0;
void loop()
{
  client.loop();

  if (!client.connected()) {
    connect();
  }

  initGag();

  if (analogRead(Pin_Reader_Fuses) > 500) // Если значение на аналоговом входе < 500
  {
    fusesFailStart = 0;
    if (!fusesOk || fusesNotSet)
    {
      digitalWrite(LED_Strip_White, HIGH);
      digitalWrite(LED_Strip_Red, LOW);
      client.publish("space/fuses/out", fusesNotSet ? OUT_FUSES_OK_SILENT : OUT_FUSES_OK);
      fusesOk = true;
      fusesNotSet = false;
    }
  }
  else
  {
    if (fusesFailStart == 0)
    {
      fusesFailStart = millis();
    }
    else
    {
      if (millis() - fusesFailStart > 500)
      {
        if (isRedOn && millis() - lastMillis > 1000)
        {
          isRedOn = false;
          digitalWrite(LED_Strip_Red, LOW);
          lastMillis = millis();
        }
        else if (!isRedOn && millis() - lastMillis > 500)
        {
          isRedOn = true;
          digitalWrite(LED_Strip_Red, HIGH);
          lastMillis = millis();
        }

        if (fusesOk || fusesNotSet)
        {
          client.publish("space/fuses/out", fusesNotSet ? OUT_FUSES_FAIL_SILENT : OUT_FUSES_FAIL);
          digitalWrite(LED_Strip_White, LOW);
          fusesNotSet = false;
          fusesOk = false;
        }
      }
    }
  }

  if (doorReady())
  {
    lcd.backlight();
    delay(50);
  }
  else
  {
    lcd.noBacklight();
    delay(50);
  }


  if (analogRead(Pin_Reader_RFID) > 500)
  {
    if ((!isDoorOpen || wasDoorOpen) && doorReady())
    {
      lcd.setCursor(0, 0);                 // Устанавливаем курсор в начало 1 строки
      lcd.print("O\1ce\2: o\1\2p\3\1");  // Отсек: открыт
      digitalWrite(Electromagnetic_lock, LOW);

      client.publish("space/console/out", "Creodoor OPEN");
      client.publish("space/creodoor/out", wasDoorOpen ? OUT_DOOR_OPEN_SILENT : OUT_DOOR_OPEN);

      isDoorOpen = true;
      wasDoorOpen = false;
      saveDoorOpen();
    }
  }

}

void initGag()
{
  if (gagNotSet && (millis() - gagLastCall > 1000))
  {
    client.publish("space/creodoor/out", OUT_GAG_REQUEST);
    gagLastCall = millis();
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  if (payload == "r")
  {
    software_Reboot();
  } else if (payload == "1")
  {
    isManual = true;
    client.publish("space/console/out", "Creodoor manual OPEN");
    digitalWrite(Electromagnetic_lock, LOW);
  } else if (payload == "0")
  {
    client.publish("space/console/out", "Creodoor manual CLOSE");
    isManual = true;
    digitalWrite(Electromagnetic_lock, HIGH);
  }
  else if (payload == IN_GAG_OPEN)
  {
    gagNotSet = false;
    gagOpen = true;
  }
  else if (payload == IN_GAG_CLOSED)
  {
    gagNotSet = false;
    gagOpen = false;
  }
  else if (payload == "i")
  {
    clearResult();
    __init();
  }
  else if (payload == "a")
  {
    client.publish("space/creodoor/out", isDoorOpen ? OUT_DOOR_OPEN_SILENT : OUT_DOOR_CLOSED_SILENT);
    client.publish("space/fuses/out", fusesOk ? OUT_FUSES_OK_SILENT : OUT_FUSES_FAIL_SILENT);
  }
  else if (payload == "p")
  {
    client.publish("space/ping/out", "creodoor");
  }
}

// DEFAULT

void connect() {
  Serial.print("connecting...");
  int n = 0;
  while (!client.connect("screo")) {
    Serial.print(".");



    delay(1000);
    n++;
    if (n > 5)
      software_Reboot();
  }

  //Serial.println("\nconnected!");
  client.subscribe("space/reset");
  client.subscribe("space/creodoor/in");
  client.subscribe("space/ping/in");
  client.subscribe("space/creodoor/reset");
  // client.unsubscribe("/example");
}

void software_Reboot()
{
  wdt_enable(WDTO_15MS);
  while (1)
  {
  }
}





