#define OUT_FINISHED "1"
#define OUT_RUNNING "0"
#define OUT_FINISHED_SILENT "9"
#define OUT_RUNNING_SILENT "8"

#include "LedControl.h"
#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <EEPROM.h>

//3, 4
//6, 8
//5, 12
//8, 6
//12, 5
//4, 3

#define LED 19

byte mac[] = { 0x55, 0xDE, 0xBE, 0x05, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 12 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

LedControl lc = LedControl(7, 6, 5, 1);
LedControl lc2 = LedControl(4, 3, 2, 2);
const int analogPin0 = 0;
const int analogPin1 = 1;
const int analogPin2 = 2;
#define a_2 2
#define b_2 1
#define resetPin 17

volatile bool ok;
volatile bool finished;
volatile bool state;

void setX(int n)
{
  int n1 = n % 10;
  int n10 = n / 10;
  lc2.setDigit(0, a_2, n10, false);
  lc2.setDigit(0, b_2, n1, false);
}

void setY(int n)
{
  int n1 = n % 10;
  int n10 = n / 10;
  lc2.setDigit(1, a_2, n10, false);
  lc2.setDigit(1, b_2, n1, false);
}

#define FIN_VAL 123

void loadResult()
{
  byte val1 = EEPROM.read(0);
  byte val2 = EEPROM.read(1);
  finished = val1 == FIN_VAL && val2 == FIN_VAL;

  if (finished)
  {
    setX(88);
    setY(88);

    lc.setDigit(0, 1, 8, false);
    lc.setDigit(0, 2, 8, false);
    lc.setDigit(0, 3, 8, false);


    //client.publish("space/sputnik/out", finished ? OUT_FINISHED_SILENT : OUT_RUNNING_SILENT);
  }
}

void saveFinished()
{
  EEPROM.write(0, FIN_VAL);
  EEPROM.write(1, FIN_VAL);
}

void clearResult()
{
  EEPROM.write(0, 0);
  EEPROM.write(1, 0);
}


void setup() {

  // initialize timer1
  finished = false;
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 6250;            // compare match register 16MHz/256/10Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12);    // 256 prescaler
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts

  pinMode(9, INPUT); // Инициализируем цифровой pin 9 как вход
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  lc.shutdown(0, false);   // функция lc.shutdown(0,true); снова его включит
  lc.setIntensity(0, 2);  // яркость свечения, доступные значения от 0 до 15
  lc.clearDisplay(0);      // Очистка дисплея

  lc2.shutdown(0, false);   // функция lc.shutdown(0,true); снова его включит
  lc2.setIntensity(0, 12);  // яркость свечения, доступные значения от 0 до 15
  lc2.clearDisplay(0);      // Очистка дисплея
  lc2.shutdown(1, false);   // функция lc.shutdown(0,true); снова его включит
  lc2.setIntensity(1, 12);  // яркость свечения, доступные значения от 0 до 15
  lc2.clearDisplay(1);      // Очистка дисплея

  Serial.begin(9600);   // Скорость работы порта
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);

  connect();

  loadResult();
}

ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
  if (finished && !ok)
  {
    state = !state;
    digitalWrite(LED, state);
  }
}


unsigned long long startTime = 0ULL;
int neededNum;
void updateNeededNum()
{
  if (millis() - startTime < 1200000)
  {
    setX(3);
    setY(4);
    neededNum = 5;
  } else if (millis() - startTime < 1500000)
  {
    setX(6);
    setY(8);
    neededNum = 10;
  } else if (millis() - startTime < 1800000)
  {
    setX(5);
    setY(12);
    neededNum = 13;
  } else if (millis() - startTime < 2100000)
  {
    setX(8);
    setY(6);
    neededNum = 10;
  } else if (millis() - startTime < 2400000)
  {
    setX(12);
    setY(5);
    neededNum = 13;
  } else
  {
    setX(4);
    setY(3);
    neededNum = 5;
  }
}

int n1 = -1, n10 = -1, n100 = -1;

bool updateNum(int newNum, int n)
{
  if (n == 1)
  {
    if (n1 < 0 || newNum == (n1 + 1) || (newNum == (n1 - 1)))
    {
      n1 = newNum;
      return true;
    }
  }
  else if (n == 10)
  {
    if (n10 < 0 || newNum == n10 + 1 || (newNum == (n10 - 1)))
    {
      n10 = newNum;
      return true;
    }
  }
  else if (n == 100)
  {
    if (n100 < 0 || newNum == (n100 + 1) % 10 || (newNum == (n100 - 1)))
    {
      n100 = newNum;
      return true;
    }
  }

  return false;
}

bool msgSent = false;
void loop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }

  if (!finished)
  {
    updateNeededNum();

    // Соответствие аналогового значения с выводящейся цифрой

    // Первый потенциометр
    int analogValue0 = analogRead(analogPin0);

    if (analogValue0 < 100 && updateNum(0, 100))
    {
      lc.setDigit(0, 3, 0, false);
    }
    if (analogValue0 > 110 && analogValue0 < 200 && updateNum(1, 100))
    {
      lc.setDigit(0, 3, 1, false);
    }
    if (analogValue0 > 210 && analogValue0 < 300 && updateNum(2, 100))
    {
      lc.setDigit(0, 3, 2, false);
    }
    if (analogValue0 > 310 && analogValue0 < 400 && updateNum(3, 100))
    {
      lc.setDigit(0, 3, 3, false);
    }
    if (analogValue0 > 410 && analogValue0 < 500 && updateNum(4, 100))
    {
      lc.setDigit(0, 3, 4, false);
    }
    if (analogValue0 > 510 && analogValue0 < 600 && updateNum(5, 100))
    {
      lc.setDigit(0, 3, 5, false);
    }
    if (analogValue0 > 610 && analogValue0 < 700 && updateNum(6, 100))
    {
      lc.setDigit(0, 3, 6, false);
    }
    if (analogValue0 > 710 && analogValue0 < 800 && updateNum(7, 100))
    {
      lc.setDigit(0, 3, 7, false);
    }
    if (analogValue0 > 810 && analogValue0 < 900 && updateNum(8, 100))
    {
      lc.setDigit(0, 3, 8, false);
    }
    if (analogValue0 > 910 && updateNum(9, 100))
    {
      lc.setDigit(0, 3, 9, false);
    }

    // Второй потенциометр
    int analogValue1 = analogRead(analogPin1);

    if (analogValue1 < 100 && updateNum(0, 10))
    {
      lc.setDigit(0, 2, 0, false);
    }
    if (analogValue1 > 110 && analogValue1 < 200 && updateNum(1, 10))
    {
      lc.setDigit(0, 2, 1, false);
    }
    if (analogValue1 > 210 && analogValue1 < 300 && updateNum(2, 10))
    {
      lc.setDigit(0, 2, 2, false);
    }
    if (analogValue1 > 310 && analogValue1 < 400 && updateNum(3, 10))
    {
      lc.setDigit(0, 2, 3, false);
    }
    if (analogValue1 > 410 && analogValue1 < 500 && updateNum(4, 10))
    {
      lc.setDigit(0, 2, 4, false);
    }
    if (analogValue1 > 510 && analogValue1 < 600 && updateNum(5, 10))
    {
      lc.setDigit(0, 2, 5, false);
    }
    if (analogValue1 > 610 && analogValue1 < 700 && updateNum(6, 10))
    {
      lc.setDigit(0, 2, 6, false);
    }
    if (analogValue1 > 710 && analogValue1 < 800 && updateNum(7, 10))
    {
      lc.setDigit(0, 2, 7, false);
    }
    if (analogValue1 > 810 && analogValue1 < 900 && updateNum(8, 10))
    {
      lc.setDigit(0, 2, 8, false);
    }
    if (analogValue1 > 910 && updateNum(9, 10))
    {
      lc.setDigit(0, 2, 9, false);
    }

    // Третий потенциометр
    int analogValue2 = analogRead(analogPin2);

    if (analogValue2 < 100 && updateNum(0, 1))
    {
      lc.setDigit(0, 1, 0, false);
    }
    if (analogValue2 > 110 && analogValue2 < 200 && updateNum(1, 1))
    {
      lc.setDigit(0, 1, 1, false);
    }
    if (analogValue2 > 210 && analogValue2 < 300 && updateNum(2, 1))
    {
      lc.setDigit(0, 1, 2, false);
    }
    if (analogValue2 > 310 && analogValue2 < 400 && updateNum(3, 1))
    {
      lc.setDigit(0, 1, 3, false);
    }
    if (analogValue2 > 410 && analogValue2 < 500 && updateNum(4, 1))
    {
      lc.setDigit(0, 1, 4, false);
    }
    if (analogValue2 > 510 && analogValue2 < 600 && updateNum(5, 1))
    {
      lc.setDigit(0, 1, 5, false);
    }
    if (analogValue2 > 610 && analogValue2 < 700 && updateNum(6, 1))
    {
      lc.setDigit(0, 1, 6, false);
    }
    if (analogValue2 > 710 && analogValue2 < 800 && updateNum(7, 1))
    {
      lc.setDigit(0, 1, 7, false);
    }
    if (analogValue2 > 810 && analogValue2 < 900 && updateNum(8, 1))
    {
      lc.setDigit(0, 1, 8, false);
    }
    if (analogValue2 > 910 && updateNum(9, 1))
    {
      lc.setDigit(0, 1, 9, false);
    }

    int sensorValue = digitalRead(9); // Задаем переменную sensorValue для считывания состояния
    // Кнопка работает на размыкание
    if (sensorValue == 0) // Если на цифровом входе нолик
    {
      if (!msgSent)
      {
        String msg = "Sputnik attempt: " + String(n100) + String(n10) + String(n1);
        client.publish("space/console/out", msg);
        msgSent = true;
      }

      if (n1 == 5 && n10 == 4 && n100 == 3)
      {
        saveFinished();
        client.publish("space/console/out", "Sputnik LED ON");        
        client.publish("space/sputnik/out", "d");
        finished = true;
        setX(88);
        setY(88);

        lc.setDigit(0, 1, 8, false);
        lc.setDigit(0, 2, 8, false);
        lc.setDigit(0, 3, 8, false);
      }
    }
    else
    {
      msgSent = false;
    }
  }

}

void __init()
{
  startTime = millis();
  finished = ok = false;
  digitalWrite(LED, LOW);
  clearResult();
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  if (payload == "r")
  {
    hard_Reboot();
  }
  else if (payload == "i")
  {
    __init();
  }
  else if (payload == "b")
  {
    ok = true;
    digitalWrite(LED, LOW);
  }
  else if (payload == "p")
  {
    client.publish("space/ping/out", "sputnik");
  }
}

// DEFAULT

void connect() {
  Serial.print("connecting...");
  int n = 0;
  while (!client.connect("ssput")) {
    Serial.print(".");
    delay(1000);
    n++;
    if (n > 5)
      hard_Reboot();
  }

  Serial.println("\nconnected!");
  client.subscribe("space/reset");
  client.subscribe("space/sputnik/in");
  client.subscribe("space/ping/in");
  client.subscribe("space/sputnik/reset");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}






