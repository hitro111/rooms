#include <SPI.h>
#include <MFRC522.h>

#define okPin 4
#define greenLed 5
#define redLed 6

#define RST_PIN 9
#define SS_PIN 10


MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

byte cardAr_1[4] = { 114, 90, 147, 251 }; //Ar
byte cardAr_2[4] = { 18, 82, 93, 253 }; //Ar
byte cardAr_3[4] = { 210, 180, 84, 253 }; //Ar
byte cardAr_4[4] = { 131, 174, 18, 1 }; //Ar

void setup() {
  pinMode(okPin, OUTPUT);
  digitalWrite(okPin, LOW);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  digitalWrite(redLed, HIGH);



  Serial.begin(9600);     // Initialize serial communications with the PC
  while (!Serial);        // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();            // Init SPI bus

  mfrc522.PCD_Init();
}

bool isOk = false;
unsigned long long lastOk = 0;

void loop() {
  // Look for new cards
  if ( !mfrc522.PICC_IsNewCardPresent()) {
    if (millis() - lastOk > 300)
    {
      if (isOk) {
        digitalWrite(okPin, LOW);
        digitalWrite(greenLed, LOW);
        digitalWrite(redLed, HIGH);
        isOk = false;
        Serial.println("NE OK");
      }
    }
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  bool result = arr_equal(cardAr_1, mfrc522.uid.uidByte, 4) ||
                arr_equal(cardAr_2, mfrc522.uid.uidByte, 4) ||
                arr_equal(cardAr_3, mfrc522.uid.uidByte, 4) ||
                arr_equal(cardAr_4, mfrc522.uid.uidByte, 4);

  if (result)
    lastOk = millis();

  if (result && !isOk) {
    digitalWrite(okPin, HIGH);
    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, LOW);
    Serial.println("OK");
    isOk = true;
  }
}
bool arr_equal(byte const *x, byte const *y, size_t n)
{
  for (size_t i = 0; i < n; i++)
    if (x[i] != y[i])
      return false;
  return true;
}

