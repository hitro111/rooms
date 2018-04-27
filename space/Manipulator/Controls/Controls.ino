#define OUT_MAINIP_OK "1"
#define OUT_MAINIP_NO "0"


#include <EEPROM.h>
#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

byte mac[] = { 0x88, 0xAD, 0xBE, 0x08, 0x08, 0x0F };
byte ip[] = { 192, 168, 0, 16 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

#define resetPin 7
#define btnLedPin 8
#define X 0
#define Y 1
#define dir1_from 200
#define dir1_to 380
#define dir2_from 420
#define dir2_to 600


// Синий 100 - 250
// Пустышка 400 - 600

bool l, r, f, b, d;
bool x = true;
bool y = true;

bool finished = false;

#define OK_VAL 123

void loadResult()
{
  byte val1 = EEPROM.read(0);
  byte val2 = EEPROM.read(1);
  finished = val1 == OK_VAL && val2 == OK_VAL;
}

void saveFinished()
{
  EEPROM.write(0, OK_VAL);
  EEPROM.write(1, OK_VAL);
}

void clearResult()
{
  EEPROM.write(0, 0);
  EEPROM.write(1, 0);
}

void __init()
{
  l = r = f = b = d = false;
  x = y = true;
  finished = false;
  loadResult();

  if (finished)
  {
    client.publish("space/manip/out", finished ? OUT_MAINIP_OK : OUT_MAINIP_NO);
    Serial.print('S');
  }
  else
  {
    Serial.print('r');
    goStart();
  }
}

void goStart()
{
  Serial.print('U');
  delay(20000);
  Serial.print('R');
  delay(50);
  Serial.print('B');
  delay(10000);
}

void setup()
{
  delay(3000);
  pinMode(btnLedPin, OUTPUT);
  digitalWrite(btnLedPin, HIGH);
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  pinMode(9, INPUT);
  Serial.begin(9600);
  Serial.print('o');
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
  connect();
  __init();
}

void connect() {
  Serial.print("connecting...");
  int n = 0;
  while (!client.connect("smanip")) {
    Serial.print(".");
    delay(1000);
    n++;
    if (n > 5)
      hard_Reboot();
  }

  Serial.println("\nconnected!");
  client.subscribe("space/reset");
  client.subscribe("space/ping/in");
  client.subscribe("space/manip/reset");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}

int xval, yval;
unsigned long dStart;

void loop()
{
  client.loop();

  if (!client.connected()) {
    connect();
  }

  if (digitalRead(9) == LOW)
  {
    if (!finished)
    {
      delay(300);
      if (digitalRead(9) == LOW && d)
      {
        finished = true;
        Serial.print('S');
        delay(50);
        Serial.println("OK");
        client.publish("space/manip/out", OUT_MAINIP_OK);
        client.publish("space/console/out", "Manipulator success");
        saveFinished();
      }
    }
  }

  if (finished)
    return;

  if (analogRead(A2) < 200)
  {
    if (!d)
    {
      client.publish("space/console/out", "Manipulator Z start");
      Serial.print('D');
      delay(50);
      d = true;
      dStart = millis();
      digitalWrite(btnLedPin, LOW);
    }
  }

  if (d)
  {
    if (millis() - dStart > 30000)
    {
      d = false;

      digitalWrite(btnLedPin, HIGH);
    }
    //return;
  }

  xval = analogRead(A0);
  yval = analogRead(A1);


  if (xval >= dir1_from && xval <= dir1_to)
  {
    if (!l)
    {
      client.publish("space/console/out", "Manipulator left");
      Serial.print('R'); //перепутано
      delay(50);
      l = true;
      r = x = false;
    }
  }
  else if (xval >= dir2_from && xval <= dir2_to)
  {
    if (!r)
    {
      client.publish("space/console/out", "Manipulator right");
      r = true;
      l = x = false;
      Serial.print('L'); //перепутано
      delay(50);
    }
  }
  else if (!x && xval > 900)
  {
    client.publish("space/console/out", "Manipulator stop X");
    x = true;
    l = r = false;
    Serial.print('X');
    delay(50);
  }

  if (yval >= dir1_from && yval <= dir1_to)
  {
    if (!f)
    {
      client.publish("space/console/out", "Manipulator fwd");
      f = true;
      b = y = false;
      Serial.print('F');
      delay(50);
    }
  }
  else if (yval >= dir2_from && yval <= dir2_to)
  {
    if (!b)
    {
      client.publish("space/console/out", "Manipulator bwd");
      b = true;
      f = y = false;
      Serial.print('B');
      delay(50);
    }
  }
  else if (!y && yval > 900)
  {
    client.publish("space/console/out", "Manipulator stop Y");
    y = true;
    f = b = false;
    Serial.print('Y');
    delay(50);
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  if (payload == "r")
  {
    hard_Reboot();
  }
  else if (payload == "i")
  {
    clearResult();
    __init();
  }
  else if (payload == "a")
  {
    client.publish("space/manip/out", finished ? OUT_MAINIP_OK : OUT_MAINIP_NO);
  }
  else if (payload == "p")
  {
    client.publish("space/ping/out", "manip");
  }
}
