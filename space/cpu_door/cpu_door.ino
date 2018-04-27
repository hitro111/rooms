#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

byte mac[] = { 0x09, 0xAD, 0xBE, 0x09, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 17 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

#define Electromagnetic_lock    8


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

void __init()
{
  lcd.clear();
  delay(50);
  lcd.setCursor(0, 0);                 // Устанавливаем курсор в начало 1 строки
  lcd.print("O\1ce\2: \4\5\2p\3\1");   // Отсек: закрыт
  digitalWrite(Electromagnetic_lock, HIGH);
}

void setup()
{
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  client.begin("192.168.0.91", net);

  pinMode(Electromagnetic_lock, OUTPUT);

  lcd.init();                          // Инициализация lcd
  lcd.backlight();                     // Включаем подсветку
  lcd.clear();

  lcd.createChar(1, letter_t);
  lcd.createChar(2, letter_k);
  lcd.createChar(3, letter_ii);
  lcd.createChar(4, letter_z);
  lcd.createChar(5, letter_a);

  delay(100);
  lcd.setCursor(0, 0);
  lcd.print("Waiting...");
  connect();
  __init();
  delay(1000);
}

void loop()
{
  client.loop();

  if (!client.connected()) {
    connect();
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  if (payload == "r")
  {
    software_Reboot();
  }
  else if (payload == "i")
  {
    __init();
  }
  else if (payload == "1")
  {
    lcd.setCursor(0, 0);                 // Устанавливаем курсор в начало 1 строки
    lcd.print("O\1ce\2: o\1\2p\3\1    ");  // Отсек: открыт
    digitalWrite(Electromagnetic_lock, LOW);
    client.publish("space/console/out", "CPU door open");
  }
  else if (payload == "p")
  {
    client.publish("space/ping/out", "cpudoor");
  }
}

// DEFAULT

void connect() {
  Serial.print("connecting...");
  int n = 0;
  while (!client.connect("scpudoor")) {
    Serial.print(".");
    delay(1000);
    n++;
    if (n > 5)
      software_Reboot();
  }

  //Serial.println("\nconnected!");
  client.subscribe("space/reset");
  client.subscribe("space/cpudoor/in");
  client.subscribe("space/ping/in");
  client.subscribe("space/cpudoor/reset");
  // client.unsubscribe("/example");
}

void software_Reboot()
{
  wdt_enable(WDTO_15MS);
  while (1)
  {
  }
}




