#define OUT_BOX_OPEN "1"
#define OUT_BOX_CLOSED "0"

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

byte mac[] = { 0x11, 0xAD, 0xBE, 0x01, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 8 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

#define MagneticLockPin         9

#define resetPin 3
#define FusePin_1      1
#define FusePin_2      2
#define ledGreen_1     7
#define ledRed_1       6
#define ledGreen_2     5
#define ledRed_2       4

#define CHANGE_DELAY 200

bool firstCall = true;
bool isOpen;
void updateState();
bool firstOn = false;
bool secondOn = false;

void __init()
{
  firstCall = true;
  isOpen = false;
  firstOn = false;
  secondOn = false;
}

void setup() {
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);

  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
  pinMode (MagneticLockPin, OUTPUT);
  pinMode (FusePin_1, OUTPUT);
  pinMode (FusePin_2, OUTPUT);
  pinMode (ledRed_1, OUTPUT);
  pinMode (ledGreen_1, OUTPUT);
  pinMode (ledRed_2, OUTPUT);
  pinMode (ledGreen_2, OUTPUT);

  digitalWrite(MagneticLockPin, LOW);

  __init();
}

unsigned long long firstOffTime = 0;
unsigned long long secondOffTime = 0;

bool isManual = false;
void loop() {

  client.loop();

  if (!client.connected()) {
    connect();
  }


  if (analogRead(FusePin_1) < 500)
  {
    firstOn = true;
    firstOffTime = 0;

    digitalWrite(ledGreen_1, HIGH);
    digitalWrite(ledRed_1, LOW);
  }
  else
  {
    if (firstOffTime == 0)
    {
      firstOffTime = millis();
    }
    else
    {
      if (millis() - firstOffTime > CHANGE_DELAY)
      {
        firstOn = false;
        digitalWrite(ledGreen_1, LOW);
        digitalWrite(ledRed_1, HIGH);
      }
    }
  }

  if (analogRead(FusePin_2) < 500)
  {
    secondOn = true;
    secondOffTime = 0;

    digitalWrite(ledGreen_2, HIGH);
    digitalWrite(ledRed_2, LOW);
  }
  else
  {
    if (secondOffTime == 0)
    {
      secondOffTime = millis();
    }
    else
    {
      if (millis() - secondOffTime > CHANGE_DELAY)
      {
        secondOn = false;
        digitalWrite(ledGreen_2, LOW);
        digitalWrite(ledRed_2, HIGH);
      }
    }
  }

  if (!isManual)
  {
    updateState();
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  if (payload == "r")
  {
    hard_Reboot();
  } else if (payload == "1")
  {
    isManual = true;
    digitalWrite(MagneticLockPin, LOW);
  } else if (payload == "0")
  {
    isManual = true;
    digitalWrite(MagneticLockPin, HIGH);
  }
  else if (payload == "i")
  {
    __init();
  }
  else if (payload == "a")
  {
    client.publish("space/creobox/out", isOpen ? OUT_BOX_OPEN : OUT_BOX_CLOSED);
  }
  else if (payload == "p")
  {
    client.publish("space/ping/out", "creobox");
  }
}

// DEFAULT

void connect() {
  int n = 0;
  while (!client.connect("sbox")) {
    delay(1000);
    n++;
    if (n > 5)
      hard_Reboot();
  }

  client.subscribe("space/reset");
  client.subscribe("space/creobox/in");
  client.subscribe("space/ping/in");
  client.subscribe("space/creobox/reset");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}

void updateState()
{
  if ((firstCall || !isOpen) && firstOn && secondOn)
  {
    isOpen = true;
    digitalWrite(MagneticLockPin, LOW);
    client.publish("space/creobox/out", OUT_BOX_OPEN);
    firstCall = false;
    client.publish("space/console/out", "Creobox OPEN");
  }
  else if ((firstCall || isOpen) && (!firstOn || !secondOn))
  {
    isOpen = false;
    digitalWrite(MagneticLockPin, HIGH);
    client.publish("space/creobox/out", OUT_BOX_CLOSED);
    firstCall = false;
    client.publish("space/console/out", "Creobox CLOSED");
  }
}
