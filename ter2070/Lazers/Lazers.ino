//#define NO_SERVER
//#define TRACE
//#define DIAGNOSTICS
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

#define LED_GB 6
#define LED_R 5

#if defined(TRACE) || defined(DIAGNOSTICS)
const int L_CT = 4;
int L[L_CT] = {4, 8, 3, 2};
int R[L_CT] = {A5, A4, A3, A2};
#else
const int L_CT = 6;
int L[L_CT] = {4, 8, 3, 2, 1, 0};
int R[L_CT] = {A5, A4, A3, A2, A1, A0};
#endif

int OFF_VAL[L_CT];
int ON_VAL[L_CT];
bool IGNORE[L_CT];
int RISE_LIMIT = 180;

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

#if defined(TRACE) || defined(DIAGNOSTICS)
  Serial.begin(9600);
  Serial.println("4 lazers only, serial enabled");
#endif

  pinMode(LED_GB, OUTPUT);
  pinMode(LED_R, OUTPUT);

  lightOn();

  for (int i = 0; i < L_CT; ++i)
  {
    pinMode(L[i], OUTPUT);
  }

  pinMode(9, INPUT);

  __init();
}

void lightOn()
{
  analogWrite(LED_GB, 100);
  analogWrite(LED_R, 100);
}

void lightOff()
{ 
  digitalWrite(LED_GB, LOW);
  digitalWrite(LED_R, LOW);
}

unsigned long long alarmTriggered = 0;
bool alarmLightOn = false;
unsigned long long alarmLightOnChanged = 0;
#define TIME_ON 500
#define TIME_OFF 200
#define ALARM_DURATION 10000
void handleLight()
{
  if (millis() < alarmTriggered + ALARM_DURATION)
  {
    lightOff();
    if (alarmLightOn && millis() - alarmLightOnChanged > TIME_ON)
    {
      alarmLightOn = false;
      alarmLightOnChanged = millis();
    }
    else if (!alarmLightOn && millis() - alarmLightOnChanged > TIME_OFF)
    {
      alarmLightOn = true;
      alarmLightOnChanged = millis();
    }

    digitalWrite(LED_R, alarmLightOn);
  }
  else
  {
    lightOn();
  }
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

#ifdef DIAGNOSTICS
bool prompted = false;
#endif

void loop() {

#ifdef DIAGNOSTICS
  lightOff();
  if (!prompted)
  {
    Serial.println("Type any symbol to read values");
    prompted = true;
  }

  if (Serial.available())
  {
    while (Serial.available()) Serial.read();
    prompted = false;
    switchLazers(false);
    delay(1000);
    Serial.println("Lazers off");
    for (int i = 0; i < L_CT; ++i)
    {
      Serial.print(i);
      Serial.print(": ");
      Serial.println(analogRead(R[i]));
    }

    switchLazers(true);
    delay(1000);
    Serial.println("Lazers on");
    for (int i = 0; i < L_CT; ++i)
    {
      Serial.print(i);
      Serial.print(": ");
      Serial.println(analogRead(R[i]));
    }
    delay(200);
  }
#endif

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
      delay(500);

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

      switchLazers(false);

      state = Stopped;
      break;
    case Stopped:
      handleLight();
      break;
    case Activating:
#ifdef TRACE
      Serial.println("Activating");
#endif

      lightOff();

      switchLazers(true);

      delay(50);
      state = Active;
      break;
    case Active:
      for (int i = 0; i < L_CT; ++i)
      {
        if (IGNORE[i])
          continue;
        if (analogRead(R[i]) > OFF_VAL[i] - RISE_LIMIT)
        {
#ifndef NO_SERVER
          String s = "lazer: ";
          s += i;
          s += " (pin: ";
          s += R[i];
          s += ") triggered alarm";
          client.publish("ter2070/logs/server", s);
#endif
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

      alarmTriggered = millis();
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
      if (payload == "1")
      {
#ifndef NO_SERVER
        client.publish("ter2070/logs/server", "lazers activating");
#endif
        state = Activating;
      }
      else if (payload == "0")
      {
#ifndef NO_SERVER
        client.publish("ter2070/logs/server", "lazers stopping");
#endif
        state = Stopping;
      }
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
