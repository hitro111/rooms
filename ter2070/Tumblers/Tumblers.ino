//TODO types

//#define TRACE
//#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <Wire.h>
#include "PCF8574.h"

PCF8574 pcf1;
PCF8574 pcf2;

bool state[16] = {-1};
#define TREMBL 30

#define ACC "ttumblr"
byte mac[] = { 0x08, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 58 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

void __init()
{
  for (int i = 0; i < 16; ++i)
  {
    state[i] = -1;
  }
}

bool pcfRead(int i)
{
  bool val;
  if (i < 8)
  {
    val = pcf1.digitalRead(i);
  }
  else
  {
    val = pcf2.digitalRead(i % 8);
  }
  return val;
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
  Serial.println("setup");
#endif

  pcf1.begin(0x25);
  pcf2.begin(0x26);

  for (int i = 0; i < 8; ++i)
  {
    pcf1.pinMode(i, INPUT_PULLUP);
    pcf2.pinMode(i, INPUT_PULLUP);
  }

  for (int i = 0; i < 16; ++i)
  {
    state[i] = pcfRead(i);
  }

  __init();
}

long getContractState()
{
  long res = 0L;
  for (long i = 0; i < 16; ++i)
  {
    if (state[i])
    {
      res = res | (1L << i);
    }
  }

  return res;
}

void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif

  bool val;
  unsigned long long lastChanged = 0;
  bool isDirty = false;
  for (int i = 0; i < 16; ++i)
  {
    val = pcfRead(i);
    if (val != state[i] && millis() - lastChanged > TREMBL)
    {
      lastChanged = millis();
      isDirty = true;
      state[i] = val;
#ifdef TRACE
      Serial.print("changed: ");
      Serial.print(i);
      Serial.print(", val: ");
      Serial.println(val);
#endif
    }
  }

  if (isDirty)
  {
    long contractState = getContractState();
#ifdef TRACE
    Serial.print("Sending: ");
    Serial.println(contractState);
#endif
#ifndef NO_SERVER
    client.publish("ter2070/ttumblr/server", String(contractState));
#endif
  }
}

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
  client.subscribe("ter2070/ttumblr/reset");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}
