#define OUT_GAG_ON "1"
#define OUT_GAG_OFF "0"
#define OUT_GAG_ON_SILENT "9"
#define OUT_GAG_OFF_SILENT "8"
#define analogPin0 0
#define analogPin1 1
#define analogPin2 2
#define analogPin3 3
#define analogPin4 4
#define analogPin5 5

#define Load_1      2

#define Led_1_OK    9
#define Led_2_OK    8
#define Led_3_OK    6
#define Led_4_OK    4
#define Led_5_OK    1
#define Led_6_OK    0  

#include <Ethernet.h>
#include <MQTTClient.h>
#include <avr/wdt.h>

#define resetPin 7

byte mac[] = { 0x22, 0xAD, 0xBE, 0x02, 0x02, 0x0F };
byte ip[] = { 192, 168, 0, 9 }; // <- change to match your network

EthernetClient net;
MQTTClient client;

unsigned long lastMillis = 0;
bool firstCall = true;
void sendState();

void green()
{
  digitalWrite(3, LOW);
  digitalWrite(5, HIGH);
}

void red()
{
  digitalWrite(5, LOW);
  digitalWrite(3, HIGH);
}

bool manualOk = false;

void __init()
{
  firstCall = true;
  manualOk = false;
}

void setup() {
  pinMode(resetPin, OUTPUT);
  pinMode (Load_1,    OUTPUT);
  pinMode (Led_1_OK , OUTPUT);
  pinMode (Led_2_OK , OUTPUT);
  pinMode (Led_3_OK , OUTPUT);
  pinMode (Led_4_OK , OUTPUT);
  pinMode (Led_5_OK , OUTPUT);
  pinMode (Led_6_OK , OUTPUT);
  digitalWrite(resetPin, LOW);
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  __init();
  Ethernet.begin(mac, ip);
  client.begin("192.168.0.91", net);
  connect();
}

void connect() {
  int n = 0;
  while (!client.connect("sgag")) {
    delay(1000);
    n++;
    if (n > 5)
      hard_Reboot();
  }
  
  client.subscribe("space/reset");
  client.subscribe("space/ping/in");
  client.subscribe("space/gagarin/in");
  client.subscribe("space/gagarin/reset");
  // client.unsubscribe("/example");
}

void hard_Reboot()
{
  digitalWrite(resetPin, HIGH);
}

unsigned long long lastUpdateTime = 0;
bool updatePending = false;
bool ok = false;
void loop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }

  if (manualOk)
    return;

  sendState();
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
    client.publish("space/gagarin/out", ok ? OUT_GAG_ON_SILENT : OUT_GAG_OFF_SILENT);
  }
  else if (payload == "p")
  {
    client.publish("space/ping/out", "gagain");
  }
  else if (payload == "1")
  {
    manualOk = true;
    green();
  }
}

void sendState()
{
  bool wasFirstCall = firstCall;
  int analogValue0 = analogRead(analogPin0); // цифра 1 // 233 либо 309
  int analogValue1 = analogRead(analogPin1); // цифра 2 // 393
  int analogValue2 = analogRead(analogPin2); // цифра 0 // 183
  int analogValue3 = analogRead(analogPin3); // цифра 4 // 456
  int analogValue4 = analogRead(analogPin4); // цифра 6 // 510
  int analogValue5 = analogRead(analogPin5); // цифра 1 // 233 либо 309

  if (
    (210 < analogValue0 && analogValue0 < 270 || // цифра 1 // 233 либо 309
     270 < analogValue0 && analogValue0 < 350) && // цифра 1 // 233 либо 309

    250 < analogValue1 && analogValue1 < 425 && // цифра 2 // 393
    100 < analogValue2 && analogValue2 < 210 && // цифра 0 // 183
    425 < analogValue3 && analogValue3 < 485 && // цифра 4 // 456
    485 < analogValue4 && analogValue4 < 600 && // цифра 6 // 510

    (210 < analogValue5 && analogValue5 < 270 ||  // цифра 1 // 233 либо 309
     270 < analogValue5 && analogValue5 < 350)    // цифра 1 // 233 либо 309
  ) {
    if (!ok || firstCall)
    {
      lastUpdateTime = millis();
      updatePending = true;
      ok = true;
      firstCall = false;
    }
  }
  else if (ok || firstCall)
  {
    lastUpdateTime = millis();
    updatePending = true;
    ok = false;
    firstCall = false;
  }

  if (updatePending && (millis() - lastUpdateTime > 200))
  {
    if (ok)
    {
      green();
      client.publish("space/gagarin/out", wasFirstCall ? OUT_GAG_ON_SILENT : OUT_GAG_ON);

    }
    else if (!ok)
    {
      red();
      client.publish("space/gagarin/out", wasFirstCall ? OUT_GAG_OFF_SILENT : OUT_GAG_OFF);
    }
    updatePending = false;
    wasFirstCall = false;
  }
  if ( // цифра 1 в первом гнезде
    (210 < analogValue0 && analogValue0 < 270 || // цифра 1 // 233 либо 309
     270 < analogValue0 && analogValue0 < 350)   // цифра 1 // 233 либо 309
  )
  {
    digitalWrite(Led_1_OK, HIGH);  
  }
  else 
  { 
    digitalWrite(Led_1_OK, LOW); 
  }

if ( // цифра 2 в втором гнезде
    250 < analogValue1 && analogValue1 < 425 // цифра 2 // 393
  )
  {
    digitalWrite(Led_2_OK, HIGH);  
  }
  else 
  {  
    digitalWrite(Led_2_OK, LOW); 
  }

  if ( // цифра 0 в третьем гнезде
    100 < analogValue2 && analogValue2 < 210 // цифра 0 // 183
  )
  {
    digitalWrite(Led_3_OK, HIGH);  
  }
  else 
  {
    digitalWrite(Led_3_OK, LOW);  
  }

  if ( // цифра 4 в четвертом гнезде
    425 < analogValue3 && analogValue3 < 485 // цифра 4 // 456
  )
  {
    digitalWrite(Led_4_OK, HIGH);  
  }
  else 
  {
    digitalWrite(Led_4_OK, LOW);   
  }

  if ( // цифра 6 в пятом гнезде
    485 < analogValue4 && analogValue4 < 600 // цифра 6 // 510
  )
  {
    digitalWrite(Led_5_OK, HIGH); 
  }
  else 
  {
    digitalWrite(Led_5_OK, LOW);  
  }

  if (  // цифра 1 в шестом гнезде
    (210 < analogValue5 && analogValue5 < 270 || // цифра 1 // 233 либо 309
     270 < analogValue5 && analogValue5 < 350)   // цифра 1 // 233 либо 309
  )
  {
    digitalWrite(Led_6_OK, HIGH);  
  }
  else 
  {
    digitalWrite(Led_6_OK, LOW);  
  }
}
