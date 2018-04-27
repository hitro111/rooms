#define SWITCH_LIMIT 200
#define BLINK_DELAY 300

const int limitSwitch_Y1 = 0;
const int limitSwitch_Y2 = 1;
const int limitSwitch_Z1 = 2;
const int limitSwitch_Z2 = 3;

const int limitSwitch_X1 = 4;
const int limitSwitch_X2 = 5;
const int EN_X = 10;
const int IN1_X = 12;
const int IN2_X = 11;
const int IN1_Y = 8;
const int IN2_Y = 7;
const int EN_Y = 9;
const int EN_Z = 6;
const int IN1_Z = 5;
const int IN2_Z = 4;
bool stop1, stop2;


unsigned long long stopZtime = 0;
bool forceStopX, forceStopY, forceStopZ = false;
void __init()
{
  stopZtime = 0;
}

void greenLight()
{
  digitalWrite(2, LOW);
  digitalWrite(3, HIGH);
}

void redLight()
{
  digitalWrite(2, HIGH);
  digitalWrite(3, LOW);
}

void noLight()
{
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
}

void X_left()
{
  if (analogRead(limitSwitch_X2) > 200)
  {
    analogWrite(EN_X, 200);
    digitalWrite(IN1_X, HIGH);
    digitalWrite(IN2_X, LOW);
  }
  else
  {

    Serial.println("NO X2");
  }
}

void X_right()
{
  if (analogRead(limitSwitch_X1) > 200)
  {

    analogWrite(EN_X, 200);
    digitalWrite(IN1_X, LOW);
    digitalWrite(IN2_X, HIGH);
  }
  else
  {

    Serial.println("NO X1");
  }
}

void X_stop()
{
  digitalWrite(IN1_X, LOW);
  digitalWrite(IN2_X, LOW);
  digitalWrite(EN_X, LOW);
}

void Y_fwd()
{
  if (analogRead(limitSwitch_Y2) > 200)
  {
    analogWrite(EN_Y, 200);
    digitalWrite(IN1_Y, LOW);
    digitalWrite(IN2_Y, HIGH);
  }
  else
  {
    Serial.println("NO Y2");
  }
}

void Y_bwd()
{
  if (analogRead(limitSwitch_Y1) > 200)
  {
    analogWrite(EN_Y, 200);
    digitalWrite(IN1_Y, HIGH);
    digitalWrite(IN2_Y, LOW);
  }
  else
  {

    Serial.println("NO Y1");
  }
}

void Y_stop()
{
  digitalWrite(IN1_Y, LOW);
  digitalWrite(IN2_Y, LOW);
  digitalWrite(EN_Y, LOW);

  delay(50);
}

void Z_up()
{
  if (analogRead(limitSwitch_Z1) > 200)
  {
    analogWrite(EN_Z, 255);
    digitalWrite(IN1_Z, LOW);
    digitalWrite(IN2_Z, HIGH);
  }
  else
  {
    Serial.println("NO Z2");
  }
}

void Z_down()
{
  if (analogRead(limitSwitch_Z2) > 200)
  {
    analogWrite(EN_Z, 255);
    digitalWrite(IN1_Z, HIGH);
    digitalWrite(IN2_Z, LOW);
  }
  else
  {

    Serial.println("NO Z1");
  }
}


bool moveUp, moveD;
void Z_stop()
{
  digitalWrite(IN1_Z, LOW);
  digitalWrite(IN2_Z, LOW);
  digitalWrite(EN_Z, LOW);

  moveUp = moveD = false;
}

void success()
{
  Z_stop();
  greenLight();
}

void setup() {
  pinMode(IN1_X, OUTPUT);
  pinMode(IN2_X, OUTPUT);
  pinMode(EN_X, OUTPUT);
  pinMode(IN1_Y, OUTPUT);
  pinMode(IN2_Y, OUTPUT);
  pinMode(EN_Y, OUTPUT);
  pinMode(IN1_Z, OUTPUT);
  pinMode(IN2_Z, OUTPUT);
  pinMode(EN_Z, OUTPUT);

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  noLight();

  Serial.begin(9600);
}

bool redLightOff;
unsigned long long lastBlink = 0;

void loop() {

  if (moveD || moveUp)
  {
    if (millis() - lastBlink > BLINK_DELAY)
    {
      lastBlink = millis();
      redLightOff = !redLightOff;
      if (redLightOff)
      {
        noLight();
      }
      else
      {
        redLight();
      }
    }
  }

  if (analogRead(limitSwitch_X2) < SWITCH_LIMIT || analogRead(limitSwitch_X1) < SWITCH_LIMIT)
  {
    if (!forceStopX)
    {
      Serial.println("STOP X");
      forceStopX = true;
      X_stop();
    }
  }

  if (analogRead(limitSwitch_X2) > SWITCH_LIMIT && analogRead(limitSwitch_X1) > SWITCH_LIMIT)
  {
    if (forceStopX)
    {
      Serial.println("OK X");
      forceStopX = false;
    }
  }


  if (analogRead(limitSwitch_Y2) < SWITCH_LIMIT || analogRead(limitSwitch_Y1) < SWITCH_LIMIT)
  {
    if (!forceStopY)
    {
      Serial.println("STOP Y");
      forceStopY = true;
      Y_stop();
    }
  }

  if (analogRead(limitSwitch_Y2) > SWITCH_LIMIT && analogRead(limitSwitch_Y1) > SWITCH_LIMIT)
  {
    if (forceStopY)
    {
      Serial.println("OK Y");
      forceStopY = false;
    }
  }

  bool stopZ2 = analogRead(limitSwitch_Z2) < SWITCH_LIMIT;
  bool stopZ1 = analogRead(limitSwitch_Z1) < SWITCH_LIMIT;
  if (stopZ1 && moveUp || stopZ2 && moveD)
  {
    if (!forceStopZ)
    {
      Serial.println("STOP Z");
      forceStopZ = true;
      Z_stop();
    }

    if (stopZ1)
    {
      redLight();
    }

    if (stopZ2)
    {
      redLight();
      redLightOff = false;
      Z_up();
      moveD = false;
      moveUp = true;
    }
  }

  if (analogRead(limitSwitch_Z2) > SWITCH_LIMIT && analogRead(limitSwitch_Z1) > SWITCH_LIMIT)
  {
    if (forceStopZ)
    {
      Serial.println("OK Z");
      forceStopZ = false;
    }
  }

  if (Serial.available() > 0)
  {
    char c = Serial.read();
    switch (c)
    {
      case 'L':
        X_left();
        break;
      case 'R':
        X_right();
        break;
      case 'X':
        X_stop();
        break;
      case 'F':
        Y_fwd();
        break;
      case 'B':
        Y_bwd();
        break;
      case 'Y':
        Y_stop();
        break;
      case 'U':
        moveUp = true;
        moveD = false;
        Z_up();
        break;
      case 'D':
        moveUp = false;
        moveD = true;
        Z_down();
        break;
      case 'Z':
        Z_stop();
        break;
      case 'S':
        success();
        break;
      case 'o':
        noLight();
        break;
      case 'r':
        redLight();
        break;
      case 'g':
        greenLight();
        break;
    }
  }
}



