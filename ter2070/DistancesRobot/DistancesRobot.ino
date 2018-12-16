#define NO_SERVER
#define resetPin 7
//#define TRACE

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>

#include <Servo.h>

SoftwareSerial mySerial(20, 2); // RX, TX

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

#define EYE_BROKEN_PIN 6
#define ARM_LED_PIN 5
#define EYES_OK_PIN 6 // <- TODO
#define ARM1_PIN 18
#define ARM2_PIN 19

#define ACC "tdistance"
byte mac[] = { 0x09, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 59 }; // <- change to match your network

#define HEAD_SRV_PIN 2

Servo headServo;          // Севра отвечающая за поворот головы (30налево 50центр 70право)

byte servoVals[AMOUNT] = {90, 60, 30, 5};

#define ACTIVATE_TIME 1000
#define DEACTIVATE_TIME 2000
#define NOISE_TIME 200

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

  randomSeed(analogRead(0));
  pinMode(EYE_BROKEN_PIN, OUTPUT);
  pinMode(EYES_OK_PIN, OUTPUT);
  pinMode(ARM_LED_PIN, OUTPUT);
  pinMode(ARM1_PIN, OUTPUT);
  pinMode(ARM2_PIN, OUTPUT);

  digitalWrite(ARM1_PIN, LOW);
  digitalWrite(ARM2_PIN, LOW);

  __init();
}

bool blinkOn;
unsigned long long blinkDelayChanged;
int blinkDelay;

void handleBrokenEye()
{
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

bool isActivated = false;
bool rising;
long curVal = 0;
bool clapOn = false;
unsigned long long clapDelayChanged;
int clapDelay;
#define EYES_OK_MAX_VAL 200000 //(50 * 1000)
#define STEP 20
#define CLAP_PERIOD 2000
void handleOkEyesAndArms()
{
  if (isActivated)
  {
    rising = false;
    curVal = EYES_OK_MAX_VAL;

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

      if (curVal <= 0)
      {
        curVal = 0;
        rising = true;
      }
    }
  }


  //analogWrite(EYES_OK_PIN, curVal / 10000);
  analogWrite(ARM_LED_PIN, curVal / 10000);
}


void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif

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
