#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0x05, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 9 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  connect();
}

void connect() {
  Serial.print("connecting...");
  int n = 0;
  while (!client.connect("arduino3", "try", "try")) {
    Serial.print(".");
    delay(1000);
    n++;
    if (n > 5)
      software_Reboot();
  }

  Serial.println("\nconnected!");
  client.subscribe("space/reset");
  client.subscribe("space/door/in");
  // client.unsubscribe("/example");
}

void software_Reboot()
{
  wdt_enable(WDTO_15MS);
  while(1)
  {
  }
}

bool ok = false;
void loop() {
  client.loop();

  if(!client.connected()) {
    connect();
  }
  // publish a message roughly every second.
  if(analogRead(A5) > 60 && analogRead(A5) < 120 &&
     analogRead(A4) > 220 && analogRead(A4) < 280 && 
     analogRead(A3) > 480 && analogRead(A3) < 540 && 
     analogRead(A2) > 150 && analogRead(A2) < 210 && 
     analogRead(A1) > 290 && analogRead(A1) < 350 && 
     analogRead(A0) > 60 && analogRead(A0) < 120) {
      if(!ok)
      {
            ok = true;
            digitalWrite(8, HIGH);
            digitalWrite(9, HIGH);
            Serial.println("ok");
            client.publish("space/gagarin/out", "1");
      }

  }
  else if (ok)
  {
    digitalWrite(9, LOW);
    digitalWrite(8, LOW);
    ok = false;
    Serial.println("ne ok");
    client.publish("space/gagarin/out", "0");
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  if (payload == "1")
  {
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
  }
  else if (payload == "0")
  {
    digitalWrite(9, LOW);
    digitalWrite(8, LOW);
  }
  else if (payload == "r")
  {
    software_Reboot();
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
