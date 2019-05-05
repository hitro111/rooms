/*
   Subscribe:
   "ter2070/tcolor/device"
      "e" => enable game
      "d" => disable game

   Publish:
   "ter2070/tcolor/server"
      "1" => success
*/
//#define TRACE
//#define NO_SERVER

#define resetPin 7

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>
#include <Bounce2.h>

#define ACC "tcolor"

#define SENSOR_IN 4


#define LED_COUNT 4

#define neededAmount 4
#define startAmount 2

#include <DFPlayer_Mini_Mp3.h>
SoftwareSerial mySerial(20, 19);
#define fail_sound 5
#define win_sound 6
#define loose_sound 7
#define lvlup_sound 8

byte mac[] = { 0x02, 0xAD, 0xBE, 0x01, 0xAD, 0xBE };
byte ip[] = { 192, 168, 0, 52 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

bool gameEnabled;

enum GameState
{
  Null,
  NotStarted,
  DisplayingColors,
  CollectingInput,
  Success,
  Failed,
  Finished
};

enum BtnState
{
  Waiting,
  Needed,
  Other
};

byte leds[] = { 9, 8, 6, 4 };
byte inputPins[] = {A0, A2, A3, A1};
Bounce inputs[] = {Bounce(), Bounce(), Bounce(), Bounce()};
byte inputLeds[] = {3, 1, 0, 2};

GameState state;
GameState prevState;

void SetState(GameState newState)
{
  prevState = state;
  state = newState;
}

byte currentAmount;

void SwitchInputLeds(bool on)
{
  for (int i = 0; i < LED_COUNT; ++i)
  {
    digitalWrite(inputLeds[i], on ? HIGH : LOW);
  }
}

void SwitchLeds(bool on)
{
  for (int i = 0; i < LED_COUNT; ++i)
  {
    digitalWrite(leds[i], on ? HIGH : LOW);
  }
}

void SwitchAll(bool on)
{
  SwitchInputLeds(on);
  SwitchLeds(on);
}

bool isDown[LED_COUNT];
unsigned long long lastBtnUp[LED_COUNT];
unsigned long long lastBtnDown[LED_COUNT];

void UpdateBtns()
{
  if (state == NotStarted)
    return;
  for (int i = 0; i < LED_COUNT; ++i)
  {
    inputs[i].update();

    if (inputs[i].read() == LOW)
    {
      digitalWrite(inputLeds[i], HIGH);
      if (state == CollectingInput)
      {
        digitalWrite(leds[i], HIGH);
      }
      lastBtnDown[i] = millis();
    }
    else
    {
      digitalWrite(inputLeds[i], LOW);
      if (state == CollectingInput)
      {
        digitalWrite(leds[i], LOW);
      }
      lastBtnUp[i] = millis();
    }
  }
}

unsigned long long waitingBtnFrom;
int downDelay = 100;
BtnState NeededWasClicked(int x)
{
  //Serial.println(x);
  //Serial.println((double)waitingBtnFrom);
  for (int i = 0; i < LED_COUNT; ++i)
  {
    /*Serial.print("lastDown[");
      Serial.print(i);
      Serial.print("]=");
       Serial.println((double)lastBtnDown[i]);
      Serial.print("lastUp[");
      Serial.print(i);
      Serial.print("]=");
       Serial.println((double)lastBtnUp[i]);*/
    if (lastBtnDown[i] > waitingBtnFrom && i != x)
    {
      return Other;
    }

    if ( lastBtnDown[i] > waitingBtnFrom && lastBtnUp[i] > waitingBtnFrom && lastBtnUp[i] > lastBtnDown[i])
    {
      //Serial.println(inputPins[i]);
      mp3_play(i + 1);
      return i == x ? Needed : Other;
    }
  }

  return Waiting;
}

void __init()
{
  currentAmount = startAmount;
  SetState(Null);
  SetState(NotStarted);
  gameEnabled = false;
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

  randomSeed(analogRead(5));

  for (int i = 0; i < LED_COUNT; ++i)
  {
    inputs[i].attach(inputPins[i]);
    inputs[i].interval(10); // interval in ms
    pinMode(leds[i], OUTPUT);
    pinMode(inputLeds[i], OUTPUT);
  }

  mySerial.begin(9600);
  mp3_set_serial (mySerial);
  mp3_set_volume (40);
  delay (200);

  __init();
}

int lag = 100;
unsigned long long lastKeyIn = 0, lastKeyOut = 0;
bool isKeyIn, isKeyOut;
void UpdateKey()
{
  if (analogRead(SENSOR_IN) < 500)
  {
    if (!isKeyIn)
    {
      isKeyOut = false;
      lastKeyIn = millis();
      isKeyIn = true;
    }
  }
  else
  {
    if (!isKeyOut)
    {
      isKeyIn = false;
      lastKeyOut = millis();
      isKeyOut = true;
    }
  }

  if (lastKeyOut > lastKeyIn && millis() - lastKeyOut > lag)
  {
    if (state != Success)
    {
      state = NotStarted;
    }
  }

  if (lastKeyIn > lastKeyOut && millis() - lastKeyIn > lag)
  {
    if (state == NotStarted)
    {
      SwitchAll(false);
      state = DisplayingColors;
    }
  }
}

void ProcessGame();

void loop() {
#ifndef NO_SERVER
  client.loop();

  if (!client.connected()) {
    connect();
  }
#endif
  ProcessGame();
}

void ProcessNotStarted();
void ProcessDisplayingColors();
void ProcessCollectingInput();
void ProcessSuccess();
void ProcessFailed();
void ProcessFinished();

void ProcessGame()
{
#ifndef NO_SERVER
  if (!gameEnabled)
    return;
#endif

  UpdateBtns();
  UpdateKey();
  switch (state)
  {
    case NotStarted:
      ProcessNotStarted();
      break;
    case DisplayingColors:

      //temp!!!!!!! <- instant win
      //currentAmount = neededAmount;
      //SetState(Success);


      ProcessDisplayingColors();
      break;
    case CollectingInput:
      ProcessCollectingInput();
      break;
    case Success:
      ProcessSuccess();
      break;
    case Finished:
      ProcessFinished();
      break;
    case Failed:
      ProcessFailed();
      break;
  }
}

bool blinkOn;
unsigned long long lastBlink;
int blinkDelay = 300;
void ProcessNotStarted()
{
  if (prevState != state)
  {
    SwitchAll(false);
    SetState(state);
  }

  if (millis() - lastBlink > blinkDelay)
  {
#ifdef TRACE
    Serial.println("blink NS");
#endif
    SwitchAll(blinkOn);
    lastBlink = millis();
    blinkOn = !blinkOn;
  }
}

enum DisplayingColorsState
{
  PreDelay,
  Displaying,
  OnDelay,
  OffDelay
};

DisplayingColorsState displayState;
byte neededColors[neededAmount];
byte currentStep;
int preColorDelay = 1000;
int onDelay = 1000;
int offDelay = 600;
unsigned long long lastDisplayUpdate;
byte curLed;
void ProcessDisplayingColors()
{
  if (prevState != state)
  {
#ifdef TRACE
    Serial.println("ProcessDisplayingColors");
#endif
    for (int i = 0; i < LED_COUNT; ++i)
    {
      neededColors[i] = random(0, LED_COUNT);
    }
    currentStep = 0;
    displayState = PreDelay;
    lastDisplayUpdate = millis();

    SwitchInputLeds(true);
    SetState(state);
  }

  switch (displayState)
  {
    case PreDelay:
      if (millis() - lastDisplayUpdate > preColorDelay)
      {
        displayState = Displaying;
      }
      break;
    case Displaying:
      SwitchLeds(false);
      curLed = neededColors[currentStep];
      digitalWrite(leds[curLed], HIGH);
      mp3_play(curLed + 1);
      lastDisplayUpdate = millis();
      displayState = OnDelay;
      break;
    case OnDelay:
      if (millis() - lastDisplayUpdate > onDelay)
      {
        displayState = OffDelay;
        digitalWrite(leds[curLed], LOW);
        lastDisplayUpdate = millis();
      }
      break;
    case OffDelay:
      if (millis() - lastDisplayUpdate > offDelay)
      {
        currentStep++;
        if (currentStep < currentAmount)
        {
          displayState = Displaying;
        }
        else
        {
          SetState(CollectingInput);
        }
        lastDisplayUpdate = millis();
      }
      break;
  }

}

byte inputReadStep;
void ProcessCollectingInput()
{
  if (prevState != state)
  {
#ifdef TRACE
    Serial.println("ProcessCollectingInput");
#endif
    inputReadStep = 0;
    SetState(state);
    SwitchInputLeds(false);
    waitingBtnFrom = millis();
  }

  switch (NeededWasClicked(neededColors[inputReadStep]))
  {
    case Needed:
      inputReadStep++;
      waitingBtnFrom = millis();
      if (inputReadStep >= currentAmount)
      {
        SetState(Success);
      }
      break;
    case Other:
      mp3_play(fail_sound);
      SetState(Failed);
      break;
  }
}


enum ProcessSuccessState
{
  Pre,
  Event,
  Post
};

ProcessSuccessState successState;
unsigned long long successUpdate = 0;
int preDelay = 200;
int postDelay = 3000;

void ProcessSuccess()
{
#ifdef TRACE
  //Serial.println("ProcessSuccess");
#endif

  if (prevState != state)
  {
    successState = Pre;
    successUpdate = millis();
    SetState(state);
  }

  switch (successState)
  {
    case Pre:
      if (millis() - successUpdate > preDelay)
      {
        successState = Event;
        successUpdate = millis();
      }
      break;
    case Event:
      SwitchAll(true);
      if (currentAmount == neededAmount)
      {
        mp3_play(win_sound);
#ifndef NO_SERVER
        client.publish("ter2070/tcolor/server", "1");
#endif
      }
      else
      {
        mp3_play(lvlup_sound);
      }

      successState = Post;
      successUpdate = millis();
      break;
    case Post:
      if (millis() - successUpdate > postDelay)
      {
        if (currentAmount == neededAmount)
        {
          SetState(Finished);
          gameEnabled = false;
          //Success!
        }
        else
        {
          SwitchAll(false);
          currentAmount++;
          SetState(DisplayingColors);
        }
      }
      break;
  }
}

void ProcessFinished()
{
  __init();
  gameEnabled = true; //init game disabled, we need enabled (override)

#ifdef TRACE
  Serial.println("ProcessFinished exit");
#endif
}

void ProcessFailed()
{
#ifdef TRACE
  Serial.println("ProcessFailed");
#endif
  SwitchAll(true);
  delay(300);
  mp3_play(loose_sound);
  SwitchAll(false);
  delay(300);
  SwitchAll(true);
  delay(200);
  SwitchAll(false);
  delay(200);
  SwitchAll(true);
  delay(100);
  SwitchAll(false);
  delay(100);
  SwitchAll(true);
  delay(50);
  SwitchAll(false);
  delay(50);
  SwitchAll(true);
  delay(25);
  SwitchAll(false);
  delay(25);
  currentAmount = startAmount;
  SetState(DisplayingColors);
}


void messageReceived(String topic, String payload, char * bytes, unsigned int length)
{
  /*
    #ifdef TRACE
    Serial.print("Topic: ");
    Serial.print(topic);
    Serial.print(" , payload: ");
    Serial.println(payload);
    #endif
  */
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


    if (topic == "ter2070/tcolor/device")
    {

      if (payload == "e")
      {
        gameEnabled = true;
      }
      else if (payload == "d")
      {
        gameEnabled = false;
      }
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


#ifdef TRACE
  //client.subscribe("#");
#endif
  client.subscribe("ter2070/reset");
  client.subscribe("ter2070/ping/in");
  client.subscribe("ter2070/tcolor/reset");
  client.subscribe("ter2070/tcolor/device");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}
