// This example uses an Arduino Uno together with
// an Ethernet Shield to connect to shiftr.io.
//
// You can check on your device after a successful
// connection here: https://shiftr.io/try.
//
// by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt

#include <Ethernet.h>
#include <MQTTClient.h>

byte mac[] = { 0xDE, 0xAD, 0x11, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 9 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);

  connect();
}

void connect() {
  Serial.print("connecting...");
  while (!client.connect("arduino2", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  //client.subscribe("space/door/in");
  // client.unsubscribe("/example");
}

bool ok = false;
void loop() {
  client.loop();

  if(!client.connected()) {
    connect();
  }

    if(analogRead(A4) > 210 && analogRead(A4) < 280 && 
     analogRead(A3) > 140 && analogRead(A3) < 210 && 
     analogRead(A2) > 50 && analogRead(A2) < 130 && 
     analogRead(A1) > 50 && analogRead(A1) < 130 && 
     analogRead(A0) > 480 && analogRead(A0) < 540) {
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
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}
