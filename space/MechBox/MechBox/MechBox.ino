#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RST_PIN 9 // 
#define SS_PIN 10 //

#define IS_OPEN_SIGNAL 8
#define LOCK 14
#define PWD_SIZE 4

LiquidCrystal_I2C lcd(0x27, 20, 4);

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

byte card_1[4] = { 226, 193, 90, 253 };
byte card_2[4] = { 178, 254, 84, 253 };
byte card_3[4] = { 195, 253, 12, 1 };

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6, 7}; //connect to the column pinouts of the keypad

char pwd[PWD_SIZE] = {0};

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

int curPos = 0;
int ct = 0;
bool isOk = false;

void resetAll()
{
  isOk = false;
  lcd.clear();
  curPos = 0;
  for (int i = 0; i < PWD_SIZE; ++i)
    pwd[i] = 0;

  digitalWrite(IS_OPEN_SIGNAL, LOW);
  digitalWrite(LOCK, HIGH);
}

void writeWrongCard()
{
  lcd.setCursor(0, 1);
  lcd.print("OTKA3           ");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void writeOk()
{
  lcd.setCursor(0, 1);
  lcd.print("OK              ");
}

void setup() {
  lcd.init();                       // Инициализация lcd
  lcd.backlight();                  // Включаем подсветку
  delay (100);
  lcd.clear();
  SPI.begin();            // Init SPI bus
  mfrc522.PCD_Init();

  pinMode(LOCK, OUTPUT);
  pinMode(IS_OPEN_SIGNAL, OUTPUT);

  customKeypad.addEventListener(keypadEvent);
  customKeypad.setHoldTime(5000);

}

void loop() {
  char customKey = customKeypad.getKey();

  if (isOk)
    return;

  if (customKey) {
    if (curPos == PWD_SIZE)
    {
      resetAll();
    }
    else
    {
      pwd[curPos] = customKey;

      lcd.setCursor(curPos, 0);
      lcd.print(customKey);
      curPos++;
    }
  }

  // Look for new cards
  if ( !mfrc522.PICC_IsNewCardPresent()) {
    //return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  bool res = arr_equal(card_1, mfrc522.uid.uidByte, 4) ||
             arr_equal(card_2, mfrc522.uid.uidByte, 4) ||
             arr_equal(card_3, mfrc522.uid.uidByte, 4);

  if (res)
  {
    if (pwd[0] == '9' && pwd[1] == '3' && pwd[2] == '5' && pwd[3] == '7')
    {
      writeOk();
      isOk = true;
      digitalWrite(IS_OPEN_SIGNAL, HIGH);
      digitalWrite(LOCK, LOW);
    }
    else
    {
      resetAll();
    }
  }
  else
  {
    writeWrongCard();
  }
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

bool arr_equal(byte const *x, byte const *y, size_t n)
{
  for (size_t i = 0; i < n; i++)
    if (x[i] != y[i])
      return false;
  return true;
}
