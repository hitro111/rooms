#include "LedControl.h"
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
SoftwareSerial mySerial(20, 9);

LedControl lc = LedControl(5, 6, 8, 1);
#define switch_1   3 //14
#define switch_2   4 //15
#define switch_3   17 //16
#define switch_4   16 //17
#define switch_5   15 //4
#define switch_6   14 //3
#define led_charging 2

int fullMp3 = 13; // "0013.mp3"
int expected_vals[6] = {LOW, HIGH, HIGH, HIGH, LOW, HIGH};
int needed_sounds[6] = {1, 2, 3, 4, 5, 6}; //"0001.mp3", "0002.mp3"...
int bad_sounds[6] = {7, 8, 9, 10, 11, 12}; //"0007.mp3"...
int pins[6] = {3, 4, 17, 16, 15, 14};
int vals_prev[6];
int vals[6];
bool finished = false;

void setup()
{
  pinMode(switch_1, INPUT);
  pinMode(switch_2, INPUT);
  pinMode(switch_3, INPUT);
  pinMode(switch_4, INPUT);
  pinMode(switch_5, INPUT);
  pinMode(switch_6, INPUT);
  pinMode(led_charging, OUTPUT);
  mySerial.begin(9600);

  // Initialize the MAX7219 device
  lc.shutdown(0, false); // Enable display
  lc.setIntensity(0, 10); // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0); // Clear display register

  mySerial.begin(9600);
  mp3_set_serial (mySerial);
  mp3_set_volume (40);
  delay (200);

  //mp3_play(fullMp3);

  for (int i = 0; i < 6; ++i)
  {
    vals[i] = digitalRead(pins[i]);
  }

  Serial.begin(9600);
}

void loop()
{
  bool allCorrect = true;
  for (int i = 0; i < 6; ++i)
  {
    int val = digitalRead(pins[i]);

    if (val != vals[i])
    {
      bool isCorrect = expected_vals[i] == val;
      delay(100);
      mp3_play(isCorrect ? needed_sounds[i] : bad_sounds[i]);

      vals[i] = val;
    }

    if (vals[i] != expected_vals[i])
      allCorrect = false;
  }

  if (allCorrect)
  {
    lc.setDigit(0, 0, 0, false);
    lc.setDigit(0, 1, 2, false);
    lc.setDigit(0, 2, 0, false);
    lc.setDigit(0, 3, 0, false);
  }
  else
  {
    lc.clearDisplay(0);
  }
}



//30 -> 50 -> 60 -> 35 -> 25 -> 15
//50 -> 10 -> 30 -> 15 -> 60 -> 45


