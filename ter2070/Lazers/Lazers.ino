//#define NO_SERVER
//#define TRACE
#define resetPin 7

#ifndef NO_SERVER
#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#endif

#define ACC "tlazers"
byte mac[] = { 0x05, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 55 }; // <- change to match your network

#ifndef NO_SERVER
EthernetClient net;
MQTTClient client;
#endif
/*
   1 D8 A5
  2 D4 A4
  3 D3 A3
  4 D2 A2
  5 D1 A1
  6 D0 A0
*/

#define Lamp_1CH 6
#define Lamp_2CH 5

#define L_CT 6
//
int L[L_CT] = {4, 8, 3, 2, 1, 0};
int R[L_CT] = {A5, A4, A3, A2, A1, A0};

int OFF_VAL[L_CT];
int ON_VAL[L_CT];
bool IGNORE[L_CT];
int RISE_LIMIT = 200;

enum State
{
  Stopping,
  Stopped,
  Activating,
  Active,
  Alert,

  Init
};

State state;

void __init()
{
  state = Init;
}

void setup() {
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);

#ifndef NO_SERVER
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
#endif

#ifdef TRACE
  Serial.begin(9600);
#endif

  for (int i = 0; i < L_CT; ++i)
  {
    pinMode(L[i], OUTPUT);
  }


  pinMode(9, INPUT);

  __init();
}

void lightOn()
{
  digitalWrite(Lamp_1CH, HIGH);
  digitalWrite(Lamp_2CH, HIGH);
}

void lightOff()
{
  digitalWrite(Lamp_1CH, LOW);
  digitalWrite(Lamp_2CH, LOW);
}

void switchLazers(bool on)
{
  for (int i = 0; i < L_CT; ++i)
  {
    digitalWrite(L[i], on ? HIGH : LOW);
  }
}

void setIgnore(int i)
{
  IGNORE[i] = true;
#ifndef NO_SERVER
  client.publish("ter2070/tlazers/ignore/server", String(i));
#endif
  digitalWrite(L[i], LOW);
}

void loop() {

#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif

  if (digitalRead(9) == HIGH)
  {
    state = Activating;
    delay(100);
  }

  switch (state)
  {
    case Init:
      for (int i = 0; i < L_CT; ++i)
      {
        IGNORE[i] = false;
      }

      lightOff();
      delay(100);

      switchLazers(false);
      delay(1000);

      for (int i = 0; i < L_CT; ++i)
      {
        OFF_VAL[i] = analogRead(R[i]);
      }

      switchLazers(true);
      delay(1000);

      for (int i = 0; i < L_CT; ++i)
      {
        ON_VAL[i] = analogRead(R[i]);

        if (OFF_VAL[i] - ON_VAL[i] < RISE_LIMIT)
        {
          setIgnore(i);
        }
      }

      switchLazers(false);
      delay(50);

      state = Stopping;
      break;

    case Stopping:

#ifdef TRACE
      Serial.println("Stopping");
#endif

      lightOn();
      state = Stopped;
      break;
    case Stopped:
      delay(100);
      break;
    case Activating:
#ifdef TRACE
      Serial.println("Activating");
#endif

      lightOn();

      switchLazers(true);

      delay(50);
      state = Active;
      break;
    case Active:
      for (int i = 0; i < L_CT; ++i)
      {
        if (IGNORE[i])
          continue;
        if (analogRead(R[i]) > ON_VAL[i] + RISE_LIMIT)
        {
          state = Alert;
        }
      }
      break;
    case Alert:
#ifdef TRACE
      Serial.println("Alert");
#endif

      switchLazers(false);
      
#ifndef NO_SERVER
      client.publish("ter2070/tlazers/alert/server", "1");
#endif
      
      state = Stopping;
      break;
  }

}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {

#ifndef NO_SERVER
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

    if (topic == "ter2070/tlazers/activate/device")
    {
      state = Activating;
    }

    if (topic == "ter2070/tlazers/ignore/device")
    {
      setIgnore(payload.toInt());
    }
  }
#endif
}

// DEFAULT

void connect() {
#ifndef NO_SERVER
  int n = 0;
  while (!client.connect(ACC)) {
    delay(1000);
    n++;
    if (n > 5)
      hard_Reboot();
  }
  client.subscribe("ter2070/reset");
  client.subscribe("ter2070/ping/in");
  client.subscribe("ter2070/tlazers/reset");
  client.subscribe("ter2070/tlazers/activate/device");
  client.subscribe("ter2070/tlazers/ignore/device");
#endif
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}
