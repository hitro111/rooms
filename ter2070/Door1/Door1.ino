#define TRACE
#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <nfc.h>
#include <Wire.h>

#include "LedControl.h"

LedControl lc = LedControl(9, 8, 4, 1);
#define DOOR_LOCK 5
#define KEY_LOCK 6
#define REBOOT_PIN 7
#define BTN_IN 1
#define FUSE_IN 3

#define ACC "tdoor1"
byte mac[] = { 0x07, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 57 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

NFC_Module nfc;

void __init()
{
  // Initialize the MAX7219 device
  lc.shutdown(0, false); // Enable display
  lc.setIntensity(0, 10); // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0); // Clear display register
}

void setup() {
  nfc.begin();
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);

#ifndef NO_SERVER
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
#endif

#ifdef TRACE
  Serial.begin(9600);
#endif

  uint32_t versiondata = nfc.get_version();
  if (! versiondata) {
#ifdef TRACE
    Serial.print("Didn't find PN53x board");
#endif
    while (1); // halt
  }

  nfc.SAMConfiguration();

  pinMode(DOOR_LOCK, OUTPUT);
  pinMode(KEY_LOCK, OUTPUT);
  pinMode(REBOOT_PIN, OUTPUT);
  digitalWrite (DOOR_LOCK, HIGH);
  digitalWrite (KEY_LOCK, HIGH);
  digitalWrite (REBOOT_PIN, LOW);

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
  client.subscribe("ter2070/tdoor1/reset");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}


