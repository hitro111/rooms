#define AMOUNT 4

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

#define MAX_TRIG_DST 30
#define MIN_TRIG_DST 6

void __init()
{
}

void setup() {

  Serial.begin(9600);
  
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

unsigned long long lastIn[AMOUNT] = {0}, lastOut[AMOUNT] = {0}, lastActivated[AMOUNT] = {0}, minVal, buf;

void loop() {
  for (int i = 0; i < AMOUNT; ++i)
  {
    digitalWrite(pins[i][1], HIGH);
    /* Подаем импульс на вход trig дальномера */
    delayMicroseconds(10); // равный 10 микросекундам
    digitalWrite(pins[i][1], LOW); // Отключаем
    impulseTime = pulseIn(pins[i][0], HIGH); // Замеряем длину импульса
    distance_sm = impulseTime / 58; // Пересчитываем в сантиметры

    if (distance_sm > MIN_TRIG_DST && distance_sm < MAX_TRIG_DST)
    {
      if (lastIn[i] <= lastOut[i])  //if was not in
      {
        lastIn[i] = millis();
        Serial.print(byte(i));
      }
      digitalWrite(leds[i], HIGH);
    }
    else
    {
      if (lastOut[i] <= lastIn[i]) //if was not out
      {
        lastOut[i] = millis();
        Serial.print(byte(i + 10));
      }
      digitalWrite(leds[i], LOW);
    }

    delay(20);
  }
}
