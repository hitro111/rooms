#define IN_MAIN_LIGHT_ON "1"
#define IN_MAIN_LIGHT_OFF "0"
#define IN_MAIN_LIGHT_BLINK "3"

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

#define COMMON_BLINK_DELAY 65
#define resetPin 8
#define ledPin 7

byte mac[] = { 0x22, 0xAD, 0xBE, 0x11, 0x11, 0x0F };
byte ip[] = { 192, 168, 0, 19 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

void __init()
{

}

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  pinMode(ledPin, OUTPUT);
  __init();
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
  connect();
}

void connect() {
  Serial.print("connecting...");
  int n = 0;
  while (!client.connect("smlight")) {
    Serial.print(".");
    delay(1000);
    n++;
    if (n > 5)
      hard_Reboot();
  }

  Serial.println("\nconnected!");
  client.subscribe("space/reset");

  client.subscribe("space/mlight/in");
  client.subscribe("space/ping/in");
  client.subscribe("space/mlight/reset");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}

void loop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }
}

void lightOn()
{
  digitalWrite(ledPin, LOW);
}

void lightOff()
{
  digitalWrite(ledPin, HIGH);
}

#define MIN_BLINK 65
void lightBlink()
{
  lightOn();
  int blinkTimeTotal = random(4000, 12000);
  int curBlinkTime = 0;
  int curDelay;
  bool isOn = true;
  
  while (curBlinkTime < blinkTimeTotal)
  {
    int maxBlink = blinkTimeTotal - curBlinkTime;
    curDelay = random(MIN_BLINK, maxBlink > MIN_BLINK ? maxBlink : MIN_BLINK + 1);
    curBlinkTime += curDelay;
    
    isOn = !isOn;
    if (isOn)
      lightOn();
    else
      lightOff();

    delay(curDelay);
  }
  
  lightOn();
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
  else if (payload == IN_MAIN_LIGHT_ON)
  {
    client.publish("space/console/out", "Light ON");
    lightOn();
  }
  else if (payload == IN_MAIN_LIGHT_OFF)
  {
    client.publish("space/console/out", "Light OFF");
    lightOff();
  }
  else if (payload == IN_MAIN_LIGHT_BLINK)
  {
    client.publish("space/console/out", "Light BLINK");
    lightBlink();
  }
  else if (payload == "p")
  {
    client.publish("space/ping/out", "roomlight");
  }
}

