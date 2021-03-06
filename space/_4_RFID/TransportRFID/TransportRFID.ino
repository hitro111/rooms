#define OUT_RFID_OK "1"
#define OUT_RFID_NO "0"
#define OUT_RFID_OK_SILENT "9"
#define OUT_RFID_NO_SILENT "8"

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

byte mac[] = { 0x44, 0xAD, 0xBE, 0x04, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 11 }; // <- change to match your network

#define logPin A0
#define greenLed 19
#define redLed 18

EthernetClient net;
MQTTClient client;

#define rebootPin 7
void __init();

void setup() {
  Serial.begin(9600);
  pinMode(rebootPin, OUTPUT);
  digitalWrite(rebootPin, LOW);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, HIGH);
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
  connect();

  __init();
}

void connect() {
  int n = 0;
  while (!client.connect("sair")) {
    delay(1000);
    n++;
    if (n > 5)
      hard_Reboot();
  }
  client.subscribe("space/reset");
  client.subscribe("space/ping/in");  
  client.subscribe("space/air/in");
  client.subscribe("space/air/reset");
}

void hard_Reboot()
{
  Serial.println("REBOOT");
  delay(500);
  digitalWrite(rebootPin, HIGH);
}

bool ok = false;
bool firstCall = true;
bool manual = false;

void __init()
{
  firstCall = true;
  manual = false;
}

void loop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }

  if (manual)
    return;
    
  // publish a message roughly every second.
  if (analogRead(A0) > 256) {
    if (!ok || firstCall)
    {
      firstCall = false;
      ok = true;
      digitalWrite(greenLed, HIGH);
      digitalWrite(redLed, LOW);
      client.publish("space/air/out", OUT_RFID_OK);
    }

  }
  else if (ok || firstCall)
  {
    firstCall = false;
    ok = false;
    digitalWrite(greenLed, LOW);
    digitalWrite(redLed, HIGH);
    client.publish("space/air/out", OUT_RFID_NO);
  }
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
    client.publish("space/air/out", ok ? OUT_RFID_OK_SILENT : OUT_RFID_NO_SILENT);
  }
  else if (payload == "p")
  {
    client.publish("space/ping/out", "air");
  }
  else if (payload == "o")
  {
    manual = true;
  }
}


//             511
//91
//92
//183
//250
//321

//A5: 91
//A4: 250
//A3: 508
//A2: 183
//A1: 320
//A0: 92
