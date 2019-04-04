#define NO_SERVER
#define TRACE
#include <SoftwareSerial.h>
#include <PCF8574.h>

#include <Bounce2.h>
#define SIZE 8
#define BTN A1
#define NOT_SET 250
#define LED_ON LOW
#define LED_OFF HIGH
#define LED_W 5
#define LED_R 6 //6

#define ProcessOutCircleInit_delay 100
#define ProcessOutCircleRunning_delay 50
#define ProcessInCircleInit_delay 100
#define ProcessInCircleRunning_delay 50

#define OUT_BOX_OPEN "1"
#define OUT_BOX_CLOSED "0"
#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

#define ACC "tsec"
byte mac[] = { 0x01, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD };
byte ip[] = { 192, 168, 0, 51 }; // <- change to match your network

#include <DFPlayer_Mini_Mp3.h>

SoftwareSerial mp3Serial(4, 8); // RX, TX

EthernetClient net;
MQTTClient client;

PCF8574 in; // Светодиоды 1-8
PCF8574 out; // Светодиоды 9-16
Bounce btn = Bounce();

bool firstGame = true;

bool bLightLow;

void lightOn()
{
  analogWrite(LED_W, 255);
  analogWrite(LED_R, 0);
}

void lightLow()
{
  analogWrite(LED_W, 10);
  analogWrite(LED_R, 0);
}

void lightOff()
{
  analogWrite(LED_W, LOW);
  analogWrite(LED_R, LOW);
}

unsigned long long alarmTriggered = 0;
bool alarmLightOn = false;
unsigned long long alarmLightOnChanged = 0;
#define TIME_ON 500
#define TIME_OFF 200
#define ALARM_DURATION 10000
void handleLight()
{
  if (millis() > ALARM_DURATION * 2 /*not just started*/ && millis() < alarmTriggered + ALARM_DURATION)
  {
    lightOff();
    if (alarmLightOn && millis() - alarmLightOnChanged > TIME_ON)
    {
      alarmLightOn = false;
      alarmLightOnChanged = millis();
    }
    else if (!alarmLightOn && millis() - alarmLightOnChanged > TIME_OFF)
    {
      alarmLightOn = true;
      alarmLightOnChanged = millis();
    }

    digitalWrite(LED_R, alarmLightOn);
  }
  else
  {
    bLightLow ? lightLow() : lightOn();
  }
}

enum State
{
  Waiting,
  OutCircleInit,
  OutCircleRunning,
  OutCircleFail,
  OutCircleOk,
  InCircleInit,
  InCircleRunning,
  InCircleFail,
  InCircleOk,
  Finished
};
State gameState;

void Reset();
void ProcessWaiting();
void ProcessOutCircleInit();
void ProcessOutCircleRunning();
void ProcessOutCircleFail();
void ProcessOutCircleOk();
void ProcessInCircleInit();
void ProcessInCircleRunning();
void ProcessInCircleFail();
void ProcessInCircleOk();
void ProcessFinished();
bool IsBtnClick();
void SetAllOut(bool on);
void SetAllIn(bool on);

void __init()
{
  Reset();
  SetAllIn(false);
  SetAllOut(false);
  mp3_stop();
  gameState = Waiting;
}

/*
  void playBtn()
  {

  Serial.println("btn");
  //mp3_play(3);
  }*/

void playWin()
{
  mp3_set_volume (30);
  delay(10);
  mp3_play(4);
}

void playLoose()
{
  mp3_set_volume (30);
  delay(10);
  mp3_play(5);
}

void playLoopOutCircle()
{
  mp3_set_volume (30);
  delay(10);

  mp3_play (1);
  delay (50);
  mp3_single_loop (true);
  delay (50);
  mp3_single_loop (true);
}

void playLoopInCircle()
{
  mp3_set_volume (22);
  delay(10);

  mp3_play (2);
  delay (50);
  mp3_single_loop (true);
  delay (50);
  mp3_single_loop (true);
}

void setup() {
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);

  pinMode(LED_W, OUTPUT);
  pinMode(LED_R, OUTPUT);
  bLightLow = false;
  //lightOn();

#ifdef TRACE
  Serial.begin(9600);
#endif

#ifndef NO_SERVER
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
#endif
  in.begin(0x25);
  out.begin(0x26);

  for (int i = 0; i < SIZE; ++i)
  {
    in.pinMode(i, OUTPUT);
    out.pinMode(i, OUTPUT);
  }

  btn.attach(BTN);
  btn.interval(10); // interval in ms

  mp3Serial.begin (9600);
  mp3_set_serial (mp3Serial);  //set softwareSerial for DFPlayer-mini mp3 module
  delay(100);
  mp3_set_volume (30);
  mp3_stop();

  randomSeed(analogRead(A2));
  __init();

#ifdef NO_SERVER
  __init();
  gameState = OutCircleInit;
#endif


#ifdef TRACE
      Serial.println("setup done");
#endif
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  client.publish("space/console/out", topic);
  client.publish("space/console/out", payload);



  if (topic.startsWith("ter2070"))
  {
    if (payload == "r")
    {
      hard_Reboot();
    }
    else if (payload == "i")
    {
      __init();
      firstGame = true;
    }
    else if (payload == "p")
    {
      client.publish("ter2070/ping/out", ACC);
    }
    else if (topic == "ter2070/sec/in" && payload == "1")
    {
#ifdef TRACE
      Serial.println("start..");
#endif
      __init();
      gameState = OutCircleInit;
#ifdef TRACE
      Serial.println("started!");
#endif
    }
    else if (topic == "ter2070/sec/in" && payload == "0")
    {
      
#ifdef TRACE
      Serial.println("stopping..");
#endif
      __init();
#ifdef TRACE
      Serial.println("stopped!");
#endif
    }
    else if (topic == "ter2070/tlazers/activate/device")
    {
      if (payload == "1")
      {
        bLightLow = true;
      }
      else if (payload == "0")
      {
        bLightLow = false;
      }
    }
    else if (topic == "ter2070/tlazers/alert/server")
    {
      alarmTriggered = millis();
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
  client.subscribe("ter2070/sec/reset");
  client.subscribe("ter2070/sec/in");
  client.subscribe("ter2070/tlazers/activate/device");
  client.subscribe("ter2070/tlazers/alert/server");
  // client.unsubscribe("/example");
}

bool started = false;
void loop()
{
  handleLight();
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif

  if (!started)
  {
    started = true;
#ifdef TRACE
      Serial.println("LOOP");
#endif
  }

  switch (gameState)
  {
    case Waiting:
      ProcessWaiting();
      break;
    case OutCircleInit:
      ProcessOutCircleInit();
      break;
    case OutCircleRunning:
      ProcessOutCircleRunning();
      break;
    case OutCircleFail:
      ProcessOutCircleFail();
      break;
    case OutCircleOk:
      ProcessOutCircleOk();
      break;
    case InCircleInit:
      ProcessInCircleInit();
      break;
    case InCircleRunning:
      ProcessInCircleRunning();
      break;
    case InCircleFail:
      ProcessInCircleFail();
      break;
    case InCircleOk:
      ProcessInCircleOk();
      break;
    case Finished:
      ProcessFinished();
      break;
  }
}

bool btnDown = false;
bool IsBtnClick()
{
  btn.update();

  if (btn.read() == LOW)
  {
    if (!btnDown)
    {
      btnDown = true;
      return true;
    }
  }
  else
  {
    if (btnDown)
    {
      btnDown = false;
    }
  }

  return false;
}

void ProcessWaiting()
{

}

bool ProcessOutCircleInit_entered = false;
bool ProcessOutCircleInit_on = false;
byte targetLedOut = NOT_SET;
unsigned long long ProcessOutCircleInit_last = 0;
//ProcessOutCircleInit_delay
void ProcessOutCircleInit()
{
  if (!ProcessOutCircleInit_entered)
  {
    ProcessOutCircleInit_entered = true;
  }

  if (targetLedOut == NOT_SET)
  {
    if (firstGame)
    {
      playLoopOutCircle();
      firstGame = false;
    }
    targetLedOut = random(0, SIZE);
  }

  if (IsBtnClick())
  {
    // playBtn();
    out.digitalWrite(targetLedOut, LED_OFF);
    gameState = OutCircleRunning;
  }

  if (millis() - ProcessOutCircleInit_last > ProcessOutCircleInit_delay)
  {
    if (!ProcessOutCircleInit_on)
    {
      out.digitalWrite(targetLedOut, LED_ON);
    }
    else
    {
      out.digitalWrite(targetLedOut, LED_OFF);
    }

    ProcessOutCircleInit_on = !ProcessOutCircleInit_on;
    ProcessOutCircleInit_last = millis();
  }
}

bool ProcessOutCircleRunning_entered = false;
byte currentOut = 0;
unsigned long long ProcessOutCircleRunning_last = 0;
//ProcessOutCircleRunning_delay
void ProcessOutCircleRunning()
{
  if (!ProcessOutCircleRunning_entered)
  {
    ProcessOutCircleRunning_entered = true;
    SetAllOut(false);

    out.digitalWrite(currentOut, LED_ON);
  }

  if (IsBtnClick())
  {
    //playBtn();
    if (currentOut == targetLedOut)
      gameState = OutCircleOk;
    else
      gameState = OutCircleFail;
  }

  if (millis() - ProcessOutCircleRunning_last > ProcessOutCircleRunning_delay)
  {
    out.digitalWrite(currentOut, LED_OFF);
    currentOut = ++currentOut % SIZE;
    out.digitalWrite(currentOut, LED_ON);

    ProcessOutCircleRunning_last = millis();
  }

}


void ProcessOutCircleFail()
{
  playLoose();
  out.digitalWrite(currentOut, LED_ON);
  delay(500);
  out.digitalWrite(currentOut, LED_OFF);
  bool on = false;
  float j = 200.0;
  for (int i = 0; i < 10; ++i)
  {
    j /= 1.2;
    on = !on;
    SetAllOut(on);
    delay(j);
  }

  SetAllOut(false);
  Reset();
  gameState = OutCircleInit;
}

void ProcessOutCircleOk()
{
  SetAllOut(true);
  gameState = InCircleInit;
}

bool ProcessInCircleInit_entered = false;
bool ProcessInCircleInit_on = false;
byte targetLedIn = NOT_SET;
unsigned long long ProcessInCircleInit_last = 0;
//ProcessInCircleInit_delay
void ProcessInCircleInit()
{
  if (!ProcessInCircleInit_entered)
  {
    ProcessInCircleInit_entered = true;
  }

  if (targetLedIn == NOT_SET)
  {
    playLoopInCircle();
    targetLedIn = random(0, SIZE);
  }

  if (IsBtnClick())
  {
    //playBtn();
    in.digitalWrite(targetLedIn, LED_OFF);
    gameState = InCircleRunning;
  }

  if (millis() - ProcessInCircleInit_last > ProcessInCircleInit_delay)
  {
    if (!ProcessInCircleInit_on)
    {
      in.digitalWrite(targetLedIn, LED_ON);
    }
    else
    {
      in.digitalWrite(targetLedIn, LED_OFF);
    }

    ProcessInCircleInit_on = !ProcessInCircleInit_on;
    ProcessInCircleInit_last = millis();
  }
}

bool ProcessInCircleRunning_entered = false;
byte currentIn = 0;
unsigned long long ProcessInCircleRunning_last = 0;
//ProcessInCircleRunning_delay
void ProcessInCircleRunning()
{
  if (!ProcessInCircleRunning_entered)
  {
    ProcessInCircleRunning_entered = true;
    SetAllIn(false);

    in.digitalWrite(currentIn, LED_ON);
  }

  if (IsBtnClick())
  {
    //playBtn();
    if (currentIn == targetLedIn)
      gameState = InCircleOk;
    else
      gameState = InCircleFail;
  }

  if (millis() - ProcessInCircleRunning_last > ProcessInCircleRunning_delay)
  {
    in.digitalWrite(currentIn, LED_OFF);
    currentIn = ++currentIn % SIZE;
    in.digitalWrite(currentIn, LED_ON);

    ProcessInCircleRunning_last = millis();
  }
}

void ProcessInCircleFail()
{
  playLoose();
  in.digitalWrite(currentIn, LED_ON);
  delay(500);
  in.digitalWrite(currentIn, LED_OFF);

  bool on = false;
  float j = 200.0;
  for (int i = 0; i < 10; ++i)
  {
    j /= 1.2;
    on = !on;
    SetAllIn(on);
    delay(j);
  }

  Reset();
  SetAllIn(false);
  gameState = OutCircleFail;
}

void ProcessInCircleOk()
{
  playWin();
  SetAllIn(true);
  gameState = Finished;
}

void ProcessFinished()
{
  client.publish("ter2070/sec/out", "1");

#ifdef NO_SERVER
  gameState = OutCircleInit;
#endif

#ifndef NO_SERVER
  gameState = Waiting;
#endif
}

void Reset()
{
  targetLedIn = targetLedOut = NOT_SET;
  ProcessInCircleRunning_entered = ProcessOutCircleRunning_entered = ProcessOutCircleInit_entered = ProcessInCircleInit_entered = false;
}

void SetAllOut(bool on)
{
  if (!on)
  {
    out.set();
  }
  else
  {
    out.clear();
  }
}

void SetAllIn(bool on)
{
  if (!on)
  {
    in.set();
  }
  else
  {
    in.clear();
  }
}
