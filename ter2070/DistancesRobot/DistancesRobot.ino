//#define NO_SERVER
#define resetPin 7
//#define TRACE

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>

#include <Servo.h>

SoftwareSerial mySerial(52, 9); // RX, TX

#define AMOUNT 4
#define MAX_ULL 4294967295

#define EYE_BROKEN_MIN_ON_TIME 500
#define EYE_BROKEN_MAX_ON_TIME 2000
#define EYE_BROKEN_MIN_OFF_TIME 500
#define EYE_BROKEN_MAX_OFF_TIME 10000
#define EYE_BROKEN_HIGH_MIN_VAL 10
#define EYE_BROKEN_HIGH_MAX_VAL 255
#define EYE_BROKEN_LOW_MIN_VAL 0
#define EYE_BROKEN_LOW_MAX_VAL 9

#define EYE_BROKEN_PIN 5//6
#define EYES_OK_PIN 6//3 // <- TODO

#define ARM_LED_PIN 3//5
#define ARM1_PIN A4
#define ARM2_PIN A5

#define WHITE_LED 4//2
#define BLUE_LED 8//7

#define ACC "tdistance"
byte mac[] = { 0x09, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 59 }; // <- change to match your network

#define HEAD_SRV_PIN 2//4

Servo headServo;          // Севра отвечающая за поворот головы (30налево 50центр 70право)

byte servoVals[AMOUNT] = {90, 60, 30, 5};

#define ACTIVATE_TIME 1000
#define DEACTIVATE_TIME 2000
#define NOISE_TIME 200
#define TIMEOUT 60000

bool dead = false;

unsigned long long lastIn[AMOUNT] = {0}, lastOut[AMOUNT] = {0}, lastActivated = 0, minVal, buf;
int minInd;
char cmdIn[] = {'a', 'b', 'c', 'd'};
char cmdOut[] = {'A', 'B', 'C', 'D'};

unsigned long long brokenEyeLastChanged;
int brokenEyeDelay;
bool isBrokenEyeOn = false;

EthernetClient net;
MQTTClient client;

void __init()
{
  dead = false;
}

void setup() {
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);

#ifndef NO_SERVER
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
#endif

  Serial.begin(9600);

  headServo.attach(HEAD_SRV_PIN);
  headServo.write(45); // прямо

  mySerial.begin(9600);
  mp3_set_serial (mySerial);    //set Serial for DFPlayer-mini mp3 module
  delay (100);
  mp3_set_volume (80);

  randomSeed(analogRead(0));
  pinMode(EYE_BROKEN_PIN, OUTPUT);
  pinMode(EYES_OK_PIN, OUTPUT);
  pinMode(ARM_LED_PIN, OUTPUT);
  pinMode(ARM1_PIN, OUTPUT);
  pinMode(ARM2_PIN, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, LOW);

  __init();
}

bool blinkOn;
unsigned long long blinkDelayChanged;
int blinkDelay;

void handleBrokenEye()
{
  if (dead)
    return;

  if (millis() > brokenEyeLastChanged + brokenEyeDelay)
  {
    isBrokenEyeOn = !isBrokenEyeOn;
    if (isBrokenEyeOn)
    {
      brokenEyeDelay = random(EYE_BROKEN_MIN_ON_TIME, EYE_BROKEN_MAX_ON_TIME);
    }
    else
    {
      brokenEyeDelay = random(EYE_BROKEN_MIN_OFF_TIME, EYE_BROKEN_MAX_OFF_TIME);
    }

    if (!isBrokenEyeOn)
    {
      analogWrite(EYE_BROKEN_PIN, 0);
    }
    brokenEyeLastChanged = millis();
  }


  if (isBrokenEyeOn)
  {
    if (millis() > blinkDelayChanged + blinkDelay)
    {
      blinkOn = !blinkOn;
      int val;
      random(2) ? val = random(EYE_BROKEN_HIGH_MIN_VAL, EYE_BROKEN_HIGH_MAX_VAL) : random(EYE_BROKEN_LOW_MIN_VAL, EYE_BROKEN_LOW_MAX_VAL);
      blinkOn ? analogWrite(EYE_BROKEN_PIN, val) : analogWrite(EYE_BROKEN_PIN, 0);
      blinkDelay = random(10, 200);
      blinkDelayChanged = millis();
    }
  }
}

#define LIMIT 1000
unsigned long long clapStarted = 0;
bool state = false;
void clap(bool on)
{
  if (dead)
    return;

  if (on && !state)
  {
    state = true;
    clapStarted = millis();
    digitalWrite(ARM1_PIN, HIGH);
    digitalWrite(ARM2_PIN, HIGH);
  }
  else
  {
    state = false;
    digitalWrite(ARM1_PIN, LOW);
    digitalWrite(ARM2_PIN, LOW);
  }
}

void limitClap()
{
  if (state && millis() > clapStarted + LIMIT)
  {
    state = false;
    clap(false);
  }
}


#define EYES_OK_INIT_VAL 10000
bool isActivated = false;
bool rising;
long curVal = EYES_OK_INIT_VAL;
bool clapOn = false;
unsigned long long clapDelayChanged;
int clapDelay;
#define EYES_OK_MAX_VAL 200000 //(10 * 10000)
#define EYES_OK_ACTIVE_VAL 2550000
#define STEP 20
#define CLAP_PERIOD 2000
void handleOkEyesAndArms()
{
  if (dead)
    return;

  if (isActivated)
  {
    rising = false;
    curVal = EYES_OK_ACTIVE_VAL;

    if (millis() - lastActivated < CLAP_PERIOD)
    {
      if (millis() > clapDelayChanged + clapDelay)
      {
        clapOn = !clapOn;

        clap(clapOn);

        clapDelay = random(10, 200);
        clapDelayChanged = millis();
      }
    }
    else
    {
      clap(false);
    }
  }
  else
  {
    clap(false);

    if (rising)
    {
      curVal += STEP;

      if (curVal > EYES_OK_MAX_VAL)
      {
        curVal = EYES_OK_MAX_VAL;
        rising = false;
      }
    }
    else
    {
      curVal -= STEP;

      if (curVal <= EYES_OK_INIT_VAL)
      {
        curVal = EYES_OK_INIT_VAL;
        rising = true;
      }
    }
  }


  analogWrite(EYES_OK_PIN, curVal / 10000);
  analogWrite(ARM_LED_PIN, curVal / 10000);
}

void stopAll()
{
  analogWrite(EYE_BROKEN_PIN, 0);
  analogWrite(ARM_LED_PIN, 0);
  analogWrite(EYES_OK_PIN, 0);
}

void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif

  if (dead)
    return;

  handleBrokenEye();

  byte sVal;
  while (Serial.available())
  {
    sVal = Serial.read();
    switch (sVal)
    {
      case 'a':
        lastIn[0] = millis();
        break;
      case 'A':
        lastOut[0] = millis();
        break;
      case 'b':
        lastIn[1] = millis();
        break;
      case 'B':
        lastOut[1] = millis();
        break;
      case 'c':
        lastIn[2] = millis();
        break;
      case 'C':
        lastOut[2] = millis();
        break;
      case 'd':
        lastIn[3] = millis();
        break;
      case 'D':
        lastOut[3] = millis();
        break;
    }

#ifdef TRACE
    Serial.println(sVal);
#endif
  }

  for (int i = 0; i < AMOUNT; ++i)
  {
    if (lastIn[i] > lastOut[i] && millis() - lastIn[i] > TIMEOUT)
      lastOut[i] = millis();
  }

  //bool allDeactivated = true;
  isActivated = false;
  lastActivated = 0;
  for (int i = 0; i < AMOUNT; ++i)
  {
    if (lastIn[i] > lastOut[i] && millis() - lastIn[i] > ACTIVATE_TIME) //in later than out + time passed since in > ACTIVATE_TIME
    {
      isActivated = true;

      if (lastActivated < lastIn[i])
        lastActivated = lastIn[i];
      /*
        #ifdef TRACE
            Serial.print("activated: ");
            Serial.println(i);
        #endif
      */
    }

    /*
        if (lastOut[i] != 0 && (lastOut[i] <= lastIn[i] || lastOut[i] > lastIn[i] && lastOut[i] - lastIn[i] < DEACTIVATE_TIME)) //any in > out or not much time passed after out
        {
          allDeactivated = false;
      #ifdef TRACE
          Serial.print("not deactivated: ");
          Serial.println(i);
      #endif
        }
    */
  }

  //isActivated = isActivated && !allDeactivated;

  handleOkEyesAndArms();

  if (isActivated)
  {
    minVal = MAX_ULL;
    for (int i = 0; i < AMOUNT; ++i)
    {
      if (lastOut[i] > lastIn[i])
      {
        buf = MAX_ULL;
      }
      else
      {
        buf = millis() - lastIn[i];
      }

      if (i == minInd && buf <= NOISE_TIME)
      {
        break;
      }

      if (buf > NOISE_TIME && minVal > buf)
      {
        minVal = buf;
        minInd = i;
      }
    }

#ifdef TRACE
    Serial.print("min: ");
    Serial.println(minInd);
#endif
    headServo.write(servoVals[minInd]);
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

    if (topic == "ter2070/e/gunshot" && payload == "1")
    {
      dead = true;
      death();
      stopAll();
      client.publish("ter2070/e/robotDead", "1");
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
  client.subscribe("ter2070/e/gunshot");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}

////////////////////
////////////////////
////////////////////
////////////////////
////////////////////


int a = 100;
int b = 150;
int c = 200;
int d = 250;
int w = 25;

void death() {
  KZ_2();
  //  trr();

  KZ_1();
  delay (500);
  delay (100);
  mp3_play (2);
  delay (500);
  short_circuit_1();
  short_circuit_2();
  short_circuit_1();
  delay(300);
  short_circuit_1();
  delay(300);
  short_circuit_2();
  delay(100);
  short_circuit_1();
  short_circuit_1();
  short_circuit_2();
  short_circuit_3();
  short_circuit_3();
  short_circuit_1();
  delay (100);


  mp3_play (1);
  analogWrite  (ARM_LED_PIN, 60);
  analogWrite  (EYE_BROKEN_PIN, 60);
  analogWrite  (EYES_OK_PIN, 60);
  for (int i = 60; i >= 0; i--)
  {
    headServo.write(i + 40);
    analogWrite  (ARM_LED_PIN, i + 20);
    analogWrite  (EYE_BROKEN_PIN, i);
    analogWrite  (EYES_OK_PIN, i + 20);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(WHITE_LED, HIGH);
    delay(5);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(WHITE_LED, LOW);
    delay(12);
  }
  for (int i = 20; i >= 0; i--)
  {
    headServo.write(i + 20);
    analogWrite  (ARM_LED_PIN, i);
    //    analogWrite  (EYE_BROKEN_PIN, i);
    analogWrite  (EYES_OK_PIN, i);
    delay(10);
  }
}


void shups_1() {
  digitalWrite(ARM1_PIN, HIGH);
  delay(100);
  digitalWrite(ARM2_PIN, HIGH);
  delay(200);
  digitalWrite(ARM2_PIN, LOW);
  digitalWrite(ARM1_PIN, LOW);
  delay(100);
  digitalWrite(ARM1_PIN, HIGH);
  delay(100);
  digitalWrite(ARM1_PIN, LOW);
  delay(100);
  digitalWrite(ARM1_PIN, HIGH);
  delay(100);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay(200);
}

void shups_2() {
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay(100);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay(100);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay(100);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay(100);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay(100);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay(100);
}



void short_circuit_1() {

  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(ARM_LED_PIN, LOW);
  digitalWrite(EYE_BROKEN_PIN, LOW);
  digitalWrite(EYES_OK_PIN, LOW);
  headServo.write(45); // Прямо
  delay(10);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  digitalWrite(ARM_LED_PIN, HIGH);
  digitalWrite(EYE_BROKEN_PIN, LOW);
  analogWrite(EYES_OK_PIN, 10);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);

  delay(20);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, HIGH);
  delay(20);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  digitalWrite(ARM_LED_PIN, LOW);
  digitalWrite(EYE_BROKEN_PIN, LOW);
  digitalWrite(EYES_OK_PIN, LOW);
  headServo.write(0); // Прямо
  delay(10);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, HIGH);
  delay(5);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  digitalWrite(ARM_LED_PIN, HIGH);
  digitalWrite(EYE_BROKEN_PIN, HIGH);
  analogWrite(EYES_OK_PIN, 10);
  delay(3);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, HIGH);
  delay(10);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  digitalWrite(ARM_LED_PIN, LOW);
  digitalWrite(EYE_BROKEN_PIN, LOW);
  digitalWrite(EYES_OK_PIN, LOW);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  headServo.write(90); // Прямо
  delay(5);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(ARM_LED_PIN, HIGH);
  analogWrite(EYES_OK_PIN, 10);
  delay(15);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  delay(10);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(ARM_LED_PIN, LOW);
  digitalWrite(EYE_BROKEN_PIN, HIGH);
  digitalWrite(EYES_OK_PIN, LOW);
  //  headServo.write(105); // Прямо
  delay(10);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  digitalWrite(ARM_LED_PIN, HIGH);
  analogWrite(EYES_OK_PIN, 20);

  delay(20);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, HIGH);
  delay(10);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  digitalWrite(ARM_LED_PIN, HIGH);
  analogWrite(EYES_OK_PIN, 60);
  //  headServo.write(90); // Прямо
  delay(20);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);


}

void short_circuit_2() {

  digitalWrite(BLUE_LED, HIGH);
  delay(30);
  digitalWrite(ARM_LED_PIN, LOW);
  //  digitalWrite(EYE_BROKEN_PIN, LOW);
  digitalWrite(EYES_OK_PIN, LOW);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  headServo.write(45); // Прямо
  delay(20);
  delay(40);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  digitalWrite(ARM_LED_PIN, HIGH);
  //  digitalWrite(EYE_BROKEN_PIN, HIGH);
  digitalWrite(EYES_OK_PIN, HIGH);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay(30);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, HIGH);
  delay(25);
  digitalWrite(ARM_LED_PIN, LOW);
  //  digitalWrite(EYE_BROKEN_PIN, LOW);
  digitalWrite(EYES_OK_PIN, LOW);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay(35);
  delay(30);
  digitalWrite(ARM_LED_PIN, HIGH);
  delay(20);
  delay(25);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  delay(30);
  digitalWrite(ARM_LED_PIN, HIGH);
  //  digitalWrite(EYE_BROKEN_PIN, HIGH);
  digitalWrite(EYES_OK_PIN, HIGH);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay(20);
  digitalWrite(ARM_LED_PIN, LOW);
  delay(10);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, HIGH);
  delay(20);
  delay(30);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(ARM_LED_PIN, HIGH);
  digitalWrite(ARM_LED_PIN, LOW);
  //  digitalWrite(EYE_BROKEN_PIN, LOW);
  digitalWrite(EYES_OK_PIN, LOW);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  headServo.write(90); // Прямо

}



void short_circuit_3() {

  digitalWrite(BLUE_LED, HIGH);
  delay(30);
  delay(20);
  delay(40);
  digitalWrite(ARM_LED_PIN, HIGH);
  //  digitalWrite(EYE_BROKEN_PIN, HIGH);
  digitalWrite(EYES_OK_PIN, HIGH);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay(30);
  delay(25);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  digitalWrite(ARM_LED_PIN, LOW);
  //  digitalWrite(EYE_BROKEN_PIN, LOW);
  digitalWrite(EYES_OK_PIN, LOW);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay(35);
  delay(30);
  digitalWrite(ARM_LED_PIN, HIGH);
  delay(20);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, HIGH);
  delay(25);
  delay(30);
  digitalWrite(ARM_LED_PIN, HIGH);
  //  digitalWrite(EYE_BROKEN_PIN, HIGH);
  digitalWrite(EYES_OK_PIN, HIGH);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay(20);
  digitalWrite(BLUE_LED, LOW);
  delay(10);
  delay(20);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  delay(30);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(ARM_LED_PIN, HIGH);
  digitalWrite(ARM_LED_PIN, LOW);
  //  digitalWrite(EYE_BROKEN_PIN, LOW);
  digitalWrite(EYES_OK_PIN, LOW);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  headServo.write(90); // Прямо

}




void KZ_1() {
  delay (100);
  mp3_play (4);
  digitalWrite(ARM1_PIN, HIGH);
  headServo.write(50);
  analogWrite  (ARM_LED_PIN, 60);
  analogWrite  (EYE_BROKEN_PIN, 60);
  analogWrite  (EYES_OK_PIN, 60);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  delay (40);
  analogWrite  (ARM_LED_PIN, 5);
  analogWrite  (EYES_OK_PIN, 5);
  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(WHITE_LED, LOW);
  delay (60);
  analogWrite  (ARM_LED_PIN, 60);
  analogWrite  (EYES_OK_PIN, 60);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  delay (50);
  analogWrite  (ARM_LED_PIN, 5);
  analogWrite  (EYES_OK_PIN, 5);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(ARM1_PIN, LOW);
  headServo.write(45);
  delay (400);
}




void KZ_2() {
  delay (100);
  mp3_play (5);
  headServo.write(50);
  digitalWrite(ARM1_PIN, HIGH);
  analogWrite  (ARM_LED_PIN, 255);
  analogWrite  (EYE_BROKEN_PIN, 255);
  analogWrite  (EYES_OK_PIN, 255);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  delay (40);
  analogWrite  (ARM_LED_PIN, 60);
  analogWrite  (EYES_OK_PIN, 60);
  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(WHITE_LED, LOW);
  delay (60);
  headServo.write(45);
  digitalWrite(ARM1_PIN, LOW);
  analogWrite  (ARM_LED_PIN, 255);
  analogWrite  (EYES_OK_PIN, 55);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  delay (50);
  analogWrite  (ARM_LED_PIN, 60);
  analogWrite  (EYES_OK_PIN, 60);
  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(WHITE_LED, LOW);
  headServo.write(40);
  delay (1200);

  analogWrite  (ARM_LED_PIN, 255);
  analogWrite  (EYE_BROKEN_PIN, 120);
  analogWrite  (EYES_OK_PIN, 255);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  delay (20);
  digitalWrite(ARM1_PIN, HIGH);
  analogWrite  (ARM_LED_PIN, 60);
  analogWrite  (EYES_OK_PIN, 60);
  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(WHITE_LED, LOW);
  delay (30);
  analogWrite  (ARM_LED_PIN, 120);
  analogWrite  (EYES_OK_PIN, 120);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  delay (30);
  headServo.write(50);
  analogWrite  (ARM_LED_PIN, 15);
  analogWrite  (EYES_OK_PIN, 15);
  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(WHITE_LED, LOW);
  delay (20);
  analogWrite  (ARM_LED_PIN, 255);
  analogWrite  (EYES_OK_PIN, 255);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  delay (20);
  analogWrite  (ARM_LED_PIN, 15);
  analogWrite  (EYES_OK_PIN, 15);
  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(WHITE_LED, LOW);
  delay (30);
  analogWrite  (ARM_LED_PIN, 80);
  analogWrite  (EYES_OK_PIN, 80);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  delay (30);
  digitalWrite(ARM1_PIN, LOW);
  analogWrite  (ARM_LED_PIN, 10);
  analogWrite  (EYES_OK_PIN, 10);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(WHITE_LED, LOW);
  delay (500);

}




void trr() {
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay (w);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay (w);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay (w);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay (w);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay (w);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay (w);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay (w);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay (w);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay (w);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay (w);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay (w);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay (w);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay (w);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay (w);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay (w);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay (w);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay (w);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay (w);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay (w);
  digitalWrite(ARM1_PIN, HIGH);
  digitalWrite(ARM2_PIN, HIGH);
  delay (w);
  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);
  delay (w);
}
