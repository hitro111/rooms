//637822
//#define TRACE
//#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

#define ACC "tboxsimple"
byte mac[] = { 0x04, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 54 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

#define PWD_SIZE 6
#define LOCK 2
#define LED_G 5
#define LED_R 3
#define BUZ_PIN 1

LiquidCrystal_I2C lcd(0x27, 20, 4);


const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

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

byte left[8] =
{
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
};

byte right[8] =
{
  B00011,
  B00011,
  B00011,
  B00011,
  B00011,
  B00011,
  B00011,
  B00011,
};

byte bot[8] =
{
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
};

byte rowPins[ROWS] = {17, 16, 15, 14}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 6, 8, 9}; //connect to the column pinouts of the keypad

char pwd[PWD_SIZE] = {0};

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

int curPos = 0;
int ct = 0;
bool isOk = false;

void drawLockClosed()
{
  lcd.setCursor(16, 0);
  lcd.print(" \5 ");
  lcd.setCursor(16, 1);
  lcd.print("\7 \6 ");
  lcd.setCursor(16, 2);
  lcd.print(char(0x00 + 255));
  lcd.print(char(0x00 + 255));
  lcd.print(char(0x00 + 255));
  lcd.setCursor(16, 3);
  lcd.print(char(0x00 + 255));
  lcd.print(char(0x00 + 255));
  lcd.print(char(0x00 + 255));
}

void drawLockOpen()
{
  lcd.setCursor(14, 0);
  lcd.print(" \5 ");
  lcd.setCursor(14, 1);
  lcd.print("\7 \6 ");
  lcd.setCursor(16, 2);
  lcd.print(char(0x00 + 255));
  lcd.print(char(0x00 + 255));
  lcd.print(char(0x00 + 255));
  lcd.setCursor(16, 3);
  lcd.print(char(0x00 + 255));
  lcd.print(char(0x00 + 255));
  lcd.print(char(0x00 + 255));
}

void drawLockOpening()
{
  lcd.setCursor(14, 0);
  lcd.print("     ");
  lcd.setCursor(14, 1);
  lcd.print("  \6  ");
  lcd.setCursor(16, 2);
  lcd.print(char(0x00 + 255));
  lcd.print(char(0x00 + 255));
  lcd.print(char(0x00 + 255));
  lcd.setCursor(16, 3);
  lcd.print(char(0x00 + 255));
  lcd.print(char(0x00 + 255));
  lcd.print(char(0x00 + 255));
}

void clearLockImg()
{
  lcd.setCursor(15, 0);
  lcd.print("     ");
  lcd.setCursor(15, 1);
  lcd.print("     ");
  lcd.setCursor(15, 2);
  lcd.print("     ");
  lcd.setCursor(15, 3);
  lcd.print("     ");
}


void setClosedText()
{
  red();
  lcd.setCursor(5, 2);
  lcd.print("\3\4\1p\2");
  lcd.write(byte(0));
  lcd.print("o");

  drawLockClosed();

  lcd.setCursor(0, 0);
}

void setOpenText()
{
  green();
  lcd.setCursor(5, 2);
  lcd.print("o");
  lcd.write(byte(0));
  lcd.print("\1p\2");
  lcd.write(byte(0));
  lcd.print("o");

  drawLockOpening();

  delay(300);

  drawLockOpen();

  lcd.setCursor(0, 0);
}

void resetAll()
{

  isOk = false;
  lcd.clear();
  curPos = 0;
  for (int i = 0; i < PWD_SIZE; ++i)
    pwd[i] = 0;

  setClosedText();

  digitalWrite(LOCK, HIGH);
}

void writeWrongPwd()
{
  clearLockImg();

  delay(300);

  drawLockClosed();

  delay(300);

  clearLockImg();

  delay(300);

  drawLockClosed();

  lcd.setCursor(0, 0);
}

void __init()
{
  resetAll();
}

void setup() {
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);

  pinMode(LOCK, OUTPUT);
  digitalWrite(LOCK, HIGH);
  
  pinMode(BUZ_PIN, OUTPUT);
  digitalWrite(BUZ_PIN, LOW);

  lcd.init();                       // Инициализация lcd
  lcd.backlight();                  // Включаем подсветку
  lcd.createChar(0, letter_t);
  lcd.createChar(1, letter_k);
  lcd.createChar(2, letter_ii);
  lcd.createChar(3, letter_z);
  lcd.createChar(4, letter_a);
  lcd.createChar(5, bot);
  lcd.createChar(6, left);
  lcd.createChar(7, right);
  delay (100);
  lcd.clear();

  customKeypad.addEventListener(keypadEvent);
  customKeypad.setHoldTime(5000);

#ifndef NO_SERVER
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
#endif

#ifdef TRACE
  Serial.begin(9600);
#endif

  __init();
}

bool caretOn;
unsigned long long lastCaretEvent = 0;
int caretOnDelay = 300;

void green()
{
  analogWrite(LED_G, 255);
  analogWrite(LED_R, 0);
}

void red()
{
  analogWrite(LED_G, 0);
  analogWrite(LED_R, 255);
}

void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif

  char customKey = customKeypad.getKey();

  if (isOk)
    return;

  if (millis() - lastCaretEvent > caretOnDelay)
  {
    if (caretOn)
    {
      lcd.setCursor(curPos, 0);
      lcd.print(" ");
    }
    else
    {
      lcd.setCursor(curPos, 0);
      lcd.print("_");
    }
    lastCaretEvent = millis();
    caretOn = !caretOn;
  }

  if (customKey) {
    pwd[curPos] = customKey;

    lcd.setCursor(curPos, 0);
    lcd.print(customKey);

    digitalWrite(BUZ_PIN, HIGH);
    delay(5);
    digitalWrite(BUZ_PIN, LOW);

    curPos++;

    if (curPos == PWD_SIZE)
    {
      if (pwd[0] == '6' && pwd[1] == '3' && pwd[2] == '7' && pwd[3] == '8' && pwd[4] == '2' && pwd[5] == '2')
      {
        isOk = true;
        digitalWrite(LOCK, LOW);
        setOpenText();

        client.publish("ter2070/tboxsimple/out", "1");
      }
      else
      {
        digitalWrite(BUZ_PIN, HIGH);
        writeWrongPwd();
        digitalWrite(BUZ_PIN, LOW);
        resetAll();
      }
    }
  }
}

int curDig = 0;
int curBlinked = 0;
int onDelay = 600;
int offDelay = 400;
bool isOn = false;
int blinks[4] = {7, 8, 2, 4};
unsigned long long lastEvent = 0;

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
  client.subscribe("ter2070/sec/reset");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}


// Taking care of some special events.
void keypadEvent(KeypadEvent key) {
  switch (customKeypad.getState()) {
    case HOLD:
      if (key == 'C' && isOk) {
        resetAll();
      }
      break;
  }
}
