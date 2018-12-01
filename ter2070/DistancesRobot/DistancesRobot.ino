#define TRACE
#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#define AMOUNT 4

#define ACC "tdistance"
byte mac[] = { 0x09, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 59 }; // <- change to match your network

//[][0] - echo
//[][1] - trig
byte pins[AMOUNT][2] = {
  {9, 8},
  {7, 6},
  {5, 4},
  {3, 2}
};

byte leds[AMOUNT] = {14, 15, 16, 17};
unsigned int impulseTime = 0;
unsigned int distance_sm = 0;

#define MAX_TRIG_DST 150
#define MIN_TRIG_DST 6

EthernetClient net;
MQTTClient client;

void __init()
{
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
#endif

  for (int i = 0; i < AMOUNT; ++i)
  {
    pinMode(pins[i][0], INPUT); //echo
    pinMode(pins[i][1], OUTPUT); //trig
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], HIGH);
    digitalWrite(pins[i][1], LOW);
  }

  __init();
}

void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif

  for (int i = 0; i < AMOUNT; ++i)
  {
    digitalWrite(pins[i][1], HIGH);
    /* Подаем импульс на вход trig дальномера */
    delayMicroseconds(10); // равный 10 микросекундам
    digitalWrite(pins[i][1], LOW); // Отключаем
    impulseTime = pulseIn(pins[i][0], HIGH); // Замеряем длину импульса
    distance_sm = impulseTime / 58; // Пересчитываем в сантиметры
#ifdef TRACE
    Serial.print(i);
    Serial.print(": ");
    Serial.println(distance_sm);
#endif
    if (distance_sm > MIN_TRIG_DST && distance_sm < MAX_TRIG_DST)
    {
      digitalWrite(leds[i], HIGH);
    }
    else
    {
      digitalWrite(leds[i], LOW);
    }

    delay(20);
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
  client.subscribe("ter2070/tdistance/reset");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}

