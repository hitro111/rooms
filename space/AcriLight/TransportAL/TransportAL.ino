#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>


#define Red_1        'a'
#define Green_1      'b'
#define Orange_1     'c'
#define Off_1        'd'

#define Red_2        'e'
#define Green_2      'f'
#define Orange_2     'g'
#define Off_2        'h'

#define Red_3        'i'
#define Green_3      'j'
#define Orange_3     'k'
#define Off_3        'l'

#define Red_4        'm'
#define Green_4      'n'
#define Orange_4     'o'
#define Off_4        'p'

#define Red_5        'q'
#define Green_5      'r'
#define Orange_5     's'
#define Off_5        't'

#define Red_6        'v'
#define Green_6      'w'
#define Orange_6     'x'
#define Off_6        'y'

#define Test         'z'

#define All_Red      '1'
#define All_Green    '2'
#define All_Orange   '3'
#define All_Off      '4'
#define resetPin 7

byte mac[] = { 0x66, 0xAD, 0xBE, 0x06, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 13 }; // <- change to match your network

#define led1 5
#define led2 6

EthernetClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void setup() {
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  Serial.begin(9600);
  Serial.print(All_Orange);
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
  connect();
  Serial.print(All_Green);
}

void connect() {
  int n = 0;
  while (!client.connect("sacril")) {
    delay(1000);
    n++;
    if (n > 5)
      hard_Reboot();
  }
  client.subscribe("space/reset");
  client.subscribe("space/acrilight/in");
  client.subscribe("space/ping/in");
  client.subscribe("space/acrilight/reset");
  //client.subscribe("space/door/in");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}

bool ok = false;
void loop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  if (topic == "space/acrilight/in")
  {
    if (payload == "e1") //engine on
    {
      Serial.print(Green_1);
    } else if (payload == "l1") //light on
    {
      Serial.print(Green_2);
    } else if (payload == "l0") //light off
    {
      Serial.print(Red_2);
    } else if (payload == "a1") //air on
    {
      Serial.print(Green_3);
    } else if (payload == "a0") //air off
    {
      Serial.print(Red_3);
    } else if (payload == "s1") //sound on
    {
      Serial.print(Green_4);
    } else if (payload == "s0") //sound off
    {
      Serial.print(Red_4);
    } else if (payload == "f1") // fuel on
    {
      Serial.print(Green_5);
    } else if (payload == "f0") //fuel off
    {
      Serial.print(Orange_5);
    } else if (payload == "k1") //sputnik on
    {
      Serial.print(Green_6);
    } else if (payload == "z0")
    {
      Serial.print(All_Red);
      Serial.print(Orange_5);
      Serial.print(Orange_1);
    } else if (payload == "z1")
    {
      Serial.print(All_Green);
    }
  } else if (topic == "space/reset")
  {
    if (payload == "r")
    {
      hard_Reboot();
    }
  }
  else if (payload == "p")
  {
    client.publish("space/ping/out", "acrilight");
  }
}
