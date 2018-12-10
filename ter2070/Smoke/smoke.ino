#define TRACE
//#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

#define SMOKE_PIN 5
#define ON LOW
#define OFF HIGH

#define ACC "tsmoke"
byte mac[] = { 0x10, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 60 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

void __init()
{
}

void setup() {
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);

  pinMode(SMOKE_PIN, OUTPUT);
  digitalWrite(SMOKE_PIN, OFF);

#ifndef NO_SERVER
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
#endif

#ifdef TRACE
  Serial.begin(9600);
#endif

  __init();
}

void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif
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

    if (topic == "ter2070/c/smoke")
    {
      if (payload == "1")
      {
        digitalWrite(SMOKE_PIN, ON);
      }
      else if (payload == "0")
      {
        digitalWrite(SMOKE_PIN, OFF);
      }
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
  client.subscribe("ter2070/tsmoke/reset");
  client.subscribe("ter2070/c/smoke");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}
