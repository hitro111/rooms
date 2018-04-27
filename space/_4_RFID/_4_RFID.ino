#include <SPI.h>
#include <MFRC522.h>

#define logPin 3
#define RST_PIN_first 2 
#define RST_PIN_second 4 
#define RST_PIN_third 5 
#define RST_PIN_fourth 6 
#define SS_PIN_first 8 
#define SS_PIN_second 7 
#define SS_PIN_third 9 
#define SS_PIN_fourth 10 

#define LED_RFID_1 16 
#define LED_RFID_2 17 
#define LED_RFID_3 18 
#define LED_RFID_4 19

MFRC522 mfrc522_first(SS_PIN_first, RST_PIN_first); // Create MFRC522 instance
MFRC522 mfrc522_second(SS_PIN_second, RST_PIN_second);  // Create MFRC522 instance
MFRC522 mfrc522_third(SS_PIN_third, RST_PIN_third);  // Create MFRC522 instance
MFRC522 mfrc522_fourth(SS_PIN_fourth, RST_PIN_fourth);  // Create MFRC522 instance

byte cardN_1[4] = { 66, 104, 92, 253 };
byte cardN_2[4] = { 102, 184, 48, 91 };
byte cardN_3[4] = { 178, 60, 93, 253 };
byte cardN_4[4] = { 130, 156, 88, 253};
byte cardN_5[4] = { 66, 137, 87, 253 };
byte cardN_6[4] = { 83, 115, 15, 1 };

byte cardO2_1[4] = { 146, 130, 86, 253 }; //O2
byte cardO2_2[4] = { 50, 93, 90, 253 }; //O2
byte cardO2_3[4] = { 34, 75, 90, 253 }; //O2
byte cardO2_4[4] = { 82, 158, 88, 253 }; //O2

byte cardAr_1[4] = { 114, 90, 147, 251 }; //Ar
byte cardAr_2[4] = { 18, 82, 93, 253 }; //Ar
byte cardAr_3[4] = { 210, 180, 84, 253 }; //Ar
byte cardAr_4[4] = { 131, 174, 18, 1 }; //Ar

byte cardCO2_1[4] = { 242, 68, 88, 253 };
byte cardCO2_2[4] = { 178, 165, 87, 253 };
byte cardCO2_3[4] = { 51, 223, 12, 1 };
byte cardCO2_4[4] = { 18, 22, 90, 253 };

unsigned long long lastOff = 0;

void setup() {

  pinMode(LED_RFID_1, OUTPUT);
  pinMode(LED_RFID_2, OUTPUT);
  pinMode(LED_RFID_3, OUTPUT);
  pinMode(LED_RFID_4, OUTPUT);
  
  pinMode(logPin, OUTPUT);
  Serial.begin(9600);     // Initialize serial communications with the PC
  while (!Serial);        // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();            // Init SPI bus
}

int device = 0;
int wasDevice;
MFRC522 * mfrc522;
bool result[4] = {0, 0, 0, 0};
bool isOk = false;
void loop() {

  switch (device)
  {
    case 0:
      mfrc522 = &mfrc522_first;
      first_On();
      break;
    case 1:
      mfrc522 = &mfrc522_second;
      second_On();
      break;
    case 2:
      mfrc522 = &mfrc522_third;
      third_On();
      break;
    case 3:
      mfrc522 = &mfrc522_fourth;
      fourth_On();
      break;
  }

  wasDevice = device;
  device = ++device % 4;

  // Look for new cards
  if ( !mfrc522->PICC_IsNewCardPresent()) {
    //return;
  }

  // Select one of the cards
  if ( ! mfrc522->PICC_ReadCardSerial()) {
    result[wasDevice] = 0;
    return;
  }

  switch (wasDevice)
  {
    case 0:
      result[wasDevice] = arr_equal(cardN_1, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardN_2, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardN_3, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardN_4, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardN_5, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardN_6, mfrc522->uid.uidByte, 4);
      break;
    case 1:
      result[wasDevice] = arr_equal(cardO2_1, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardO2_2, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardO2_3, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardO2_4, mfrc522->uid.uidByte, 4);
      break;
    case 2:
      result[wasDevice] = arr_equal(cardAr_1, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardAr_2, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardAr_3, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardAr_4, mfrc522->uid.uidByte, 4);
      break;
    case 3:
      result[wasDevice] = arr_equal(cardCO2_1, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardCO2_2, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardCO2_3, mfrc522->uid.uidByte, 4) ||
                          arr_equal(cardCO2_4, mfrc522->uid.uidByte, 4);
      break;
  }

  if (result[0])
  {
	digitalWrite(LED_RFID_1, HIGH);
  }
  else
  {
	//первая карта не ок
  }
  
  if (result[1])
  {
	digitalWrite(LED_RFID_2, HIGH);
  }
  else
  {
	//вторая карта не ок
  }
  
  if (result[2])
  {
	digitalWrite(LED_RFID_3, HIGH);
  }
  else
  {
	//третья карта не ок
  }
  
  if (result[3])
  {
	digitalWrite(LED_RFID_4, HIGH);
  }
  else
  {
	//четвертая карта не ок
  }
  
  if (result[0] && result[1] && result[2] && result[3] && !isOk) {
    lastOff = 0;
    digitalWrite(logPin, HIGH);
    isOk = true;
  } else if (!(result[0] && result[1] && result[2] && result[3]) && isOk) {
    long long cur = millis();
    if (lastOff > 0 && cur - lastOff > 500)
    {     
      digitalWrite(logPin, LOW);
      isOk = false;
    }
    else if (lastOff == 0)
    {
      lastOff = cur;
    }
  }
}

void first_On()
{
  mfrc522_first.PCD_Init();   // Init MFRC522
}

void second_On()
{
  mfrc522_second.PCD_Init();   // Init MFRC522
}

void third_On()
{
  mfrc522_third.PCD_Init();   // Init MFRC522
}

void fourth_On()
{
  mfrc522_fourth.PCD_Init();   // Init MFRC522
}

bool arr_equal(byte const *x, byte const *y, size_t n)
{
  for (size_t i = 0; i < n; i++)
    if (x[i] != y[i])
      return false;
  return true;
}

