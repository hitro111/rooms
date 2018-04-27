#define OUT_GAG_ON "1"
#define OUT_GAG_OFF "0"
#define OUT_GAG_ON_SILENT "9"
#define OUT_GAG_OFF_SILENT "8"
#define analogPin0 0
#define analogPin1 1
#define analogPin2 2
#define analogPin3 3
#define analogPin4 4
#define analogPin5 5

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

#define resetPin 7

byte mac[] = { 0x22, 0xAD, 0xBE, 0x02, 0x02, 0x0F };
byte ip[] = { 192, 168, 0, 9 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

unsigned long lastMillis = 0;
bool firstCall = true;
void sendState();

void green()
{
  digitalWrite(5, LOW);
  digitalWrite(6, HIGH);
}

void red()
{
  digitalWrite(6, LOW);
  digitalWrite(5, HIGH);
}

bool manualOk = false;

void __init()
{
  firstCall = true;
  manualOk = false;
}

void setup() {
  Serial.begin(9600);
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  __init();
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
  connect();
}

void connect() {
  Serial.print("connecting...");
  int n = 0;
  while (!client.connect("sgag")) {
    Serial.print(".");
    delay(1000);
    n++;
    if (n > 5)
      hard_Reboot();
  }

  Serial.println("\nconnected!");
  client.subscribe("space/reset");
  client.subscribe("space/ping/in");
  client.subscribe("space/gagarin/in");
  client.subscribe("space/gagarin/reset");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}

unsigned long long lastUpdateTime = 0;
bool updatePending = false;
bool ok = false;
void loop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }

  if (manualOk)
    return;

  sendState();
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
  else if (payload == "a")
  {
    client.publish("space/gagarin/out", ok ? OUT_GAG_ON_SILENT : OUT_GAG_OFF_SILENT);
  }
  else if (payload == "p")
  {
    client.publish("space/ping/out", "gagain");
  }
  else if (payload == "1")
  {
    manualOk = true;
    green();
  }
}

void sendState()
{
  bool wasFirstCall = firstCall;
  int analogValue0 = analogRead(analogPin0); // цифра 1 // 166
  int analogValue1 = analogRead(analogPin1); // цифра 2 // 252
  int analogValue2 = analogRead(analogPin2); // цифра 0 // 89
  int analogValue3 = analogRead(analogPin3); // цифра 4 // 323
  int analogValue4 = analogRead(analogPin4); // цифра 6 // 511
  int analogValue5 = analogRead(analogPin5); // цифра 1 // 166

  if (
    130 < analogValue0 & analogValue0 < 200 & // цифра 1 // 166
    200 < analogValue1 & analogValue1 < 295 & // цифра 2 // 252
    50 < analogValue2 & analogValue2 < 130 & // цифра 0 // 89
    295 < analogValue3 & analogValue3 < 450 & // цифра 4 // 323
    450 < analogValue4 & analogValue4 < 650 & // цифра 6 // 511
    130 < analogValue5 & analogValue5 < 200 // цифра 1 // 166
  ) {
    if (!ok || firstCall)
    {
      lastUpdateTime = millis();
      updatePending = true;
      ok = true;
      firstCall = false;
    }
  }
  else if (ok || firstCall)
  {
    lastUpdateTime = millis();
    updatePending = true;
    ok = false;
    firstCall = false;
  }

  if (updatePending && (millis() - lastUpdateTime > 200))
  {
    if (ok)
    {
      green();
      client.publish("space/gagarin/out", wasFirstCall ? OUT_GAG_ON_SILENT : OUT_GAG_ON);

    }
    else if (!ok)
    {
      red();
      client.publish("space/gagarin/out", wasFirstCall ? OUT_GAG_OFF_SILENT : OUT_GAG_OFF);
    }
    updatePending = false;
    wasFirstCall = false;
  }
}

