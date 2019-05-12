#define TRACE
//#define NO_SERVER
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

#define ACC "tlight"
byte mac[] = { 0x17, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 67 }; // <- change to match your network

#define    LED_lightening       5
#define    LED_lightening2      6
#define    Load                 13

EthernetClient net;
MQTTClient client;

#define GEN_INIT_VAL 6

int lightVals[100] = {0, 1, 2, 3, 5, 5, 7, 8, 8, 11, 12, 12, 15, 15, 16, 18, 18, 18, 21, 23, 23, 23, 26, 29, 29, 31, 32, 34, 34, 37, 42, 42, 42, 42, 45, 45, 48, 50, 50, 50, 50, 53, 56, 58, 61, 61, 61, 69, 69, 69, 69, 77, 77, 80, 80, 85, 85, 88, 88, 88, 96, 96, 104, 104, 106, 111, 111, 116, 116, 123, 123, 128, 128, 135, 140, 140, 145, 145, 152, 157, 157, 162, 169, 169, 174, 183, 186, 186, 193, 198, 203, 208, 215, 215, 227, 227, 236, 239, 248, 255};

bool genOn = false;
bool shotOk = false;
int lightVal = 0;
void __init()
{
  genOn = false;
  shotOk = false;
  lightVal = lightVals[GEN_INIT_VAL];
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


  pinMode(LED_lightening, OUTPUT);
  pinMode(LED_lightening2, OUTPUT);
  pinMode(Load, OUTPUT);
  digitalWrite(LED_lightening, HIGH);
  digitalWrite(LED_lightening2, HIGH);
  digitalWrite(Load, HIGH);

  __init();
}

void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif

  if (!shotOk)
  {
    if (genOn)
    {
      analogWrite  (LED_lightening, lightVal);
      analogWrite  (LED_lightening2, lightVal);
    }
    else
    {
      analogWrite  (LED_lightening, 0);
      analogWrite  (LED_lightening2, 0);
    }
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  if (topic.startsWith("ter"))
  {

    if (topic.equals("ter2070/e/genOn"))
    {
      if (payload == "0")
      {
        genOn = false;
      }
      else if (payload == "1")
      {
        genOn = true;
      }
    }

    if (topic.equals("ter2070/e/genPwr"))
    {
      if (!shotOk)
      {
        payload.remove(0, 1);
        int genVal = payload.toInt();
        genVal = genVal >= 100 ? 99 : genVal;
        lightVal = lightVals[genVal];
      }
    }

    if (topic.equals("ter2070/e/gunshot"))
    {
      if (payload == "0")
      {
        charging();
      }
      else if (payload == "1")
      {
        shotDone();
      }
      else if (payload == "2")
      {
        shotFailed();
      }
      else if (payload == "3")
      {
        shotOk = true;
        shotStarted();
      }
    }

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
  client.subscribe("ter2070/e/gunshot");
  client.subscribe("ter2070/e/genOn");
  client.subscribe("ter2070/e/genPwr");
  // client.unsubscribe("/example");
}

void charging()
{
#ifdef TRACE
  Serial.println("CHARGING");
#endif
  gun_charging();
}

void shotDone()
{
#ifdef TRACE
  Serial.println("SHOT DONE");
#endif
}

void shotFailed()
{
#ifdef TRACE
  Serial.println("SHOT FAILED");
#endif
  gun_fire_fucked_up();
}

void shotStarted()
{
#ifdef TRACE
  Serial.println("OK SHOT START");
#endif
  gun_fire();
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}


void neon_1() {
  delay (300);
  delay (100);
  delay (50);
  delay (100);
  delay (50);
  delay (150);
  delay (100);
}




void front_fire() {
  delay (20);
  delay (20);
  delay (20);
  delay (20);
  delay (20);
  delay (20);
  delay (20);
  delay (20);
  delay (20);
  delay (20);
}

void gun_fire_fucked_up()
{
  //  Отправляем команду в скетче с ЦПУ добавляем задержку в 450
  for (int i = 255; i >= 105; i--)
  {
    analogWrite  (LED_lightening, i);
    analogWrite  (LED_lightening2, i);
    delay(3);
  }
  analogWrite  (LED_lightening, 100);
  analogWrite  (LED_lightening2, 100);
  delay (160);
  analogWrite  (LED_lightening, 50);
  analogWrite  (LED_lightening2, 50);
  delay (70);
  analogWrite  (LED_lightening, 130);
  analogWrite  (LED_lightening2, 130);
  delay (130);
  analogWrite  (LED_lightening, 100);
  analogWrite  (LED_lightening2, 100);
  delay (100);
  analogWrite  (LED_lightening, 130);
  analogWrite  (LED_lightening2, 130);
  delay (700);
  analogWrite  (LED_lightening, 70);
  analogWrite  (LED_lightening2, 70);
  delay (80);
  analogWrite  (LED_lightening, 150);
  analogWrite  (LED_lightening2, 150);
  delay (80);
  analogWrite  (LED_lightening, 80);
  analogWrite  (LED_lightening2, 80);
  delay (80);
  analogWrite  (LED_lightening, 110);
  analogWrite  (LED_lightening2, 110);
  delay (80);
  analogWrite  (LED_lightening, 100);
  analogWrite  (LED_lightening2, 100);
  delay (80);
  analogWrite  (LED_lightening, 110);
  analogWrite  (LED_lightening2, 110);
  delay (50);
  analogWrite  (LED_lightening, 80);
  analogWrite  (LED_lightening2, 80);
  delay(100);
  analogWrite  (LED_lightening, 100);
  analogWrite  (LED_lightening2, 100);
  for (int i = 105; i < 255; i++)         //Добавить задержку 750 в основной скетч
  {
    analogWrite  (LED_lightening, i);
    analogWrite  (LED_lightening2, i);
    delay(5);
  }
}

void gun_charging() {
  //  Отправляем команду в скетче с ЦПУ добавляем задержку в 750
  for (int i = 255; i >= 105; i--)
  {
    analogWrite  (LED_lightening, i);
    analogWrite  (LED_lightening2, i);
    delay(5);
  }
  delay (100);
  neon_1();
  analogWrite  (LED_lightening, 100);
  analogWrite  (LED_lightening2, 100);
  neon_1();
  analogWrite  (LED_lightening, 90);
  analogWrite  (LED_lightening2, 90);
  neon_1();
  analogWrite  (LED_lightening, 80);
  analogWrite  (LED_lightening2, 80);
  neon_1();
  analogWrite  (LED_lightening, 70);
  analogWrite  (LED_lightening2, 70);
  neon_1();
  analogWrite  (LED_lightening, 60);
  analogWrite  (LED_lightening2, 60);
  delay (200);
  delay (100);
}

void gun_fire() {
  analogWrite  (LED_lightening, 0);
  analogWrite  (LED_lightening2, 0);

  /*
    analogWrite  (LED_lightening, 60);
    analogWrite  (LED_lightening2, 60);
    delay (100);
    for (int i = 10; i < 50; i++)
    {
    analogWrite  (LED_lightening, 55 - i);
    analogWrite  (LED_lightening2, 55 - i);
    delay(30);
    }
    for (int i = 50; i < 255; i++)
    {
    analogWrite  (Load, i);
    delay(4);
    }
    delay(100);
    front_fire();
    front_fire();
    front_fire();
    front_fire();
    front_fire();
    front_fire();
    front_fire();
    for (int i = 255; i >= 0; i--)
    {
    analogWrite  (Load, i);
    delay(3);
    }

    // ПОИГРАТьСЯ И ДОБАВИТЬ ВОСТАНОВЛЕНИЕ ПОДСВЕТКИ
  */
}
