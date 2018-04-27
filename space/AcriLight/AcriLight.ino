
int val;


#define pwm    25

#define RedLed_1      13
#define RedLed_2      12
#define RedLed_3      8
#define RedLed_4      7
#define RedLed_5      4
#define RedLed_6      2

#define GreenLed_1      11
#define GreenLed_2      10
#define GreenLed_3      9
#define GreenLed_4      6
#define GreenLed_5      5
#define GreenLed_6      3



#define Red_1        'a'
#define Green_1      'b'
#define Orange_1     'c'
#define Off_1        'd'

#define Red_2        'e'
#define Green_2      'f'
#define Orange_2     'g'
#define Off_2        'h'

#define Red_3        'i'
#define Green_3      'j'
#define Orange_3     'k'
#define Off_3        'l'

#define Red_4        'm'
#define Green_4      'n'
#define Orange_4     'o'
#define Off_4        'p'

#define Red_5        'q'
#define Green_5      'r'
#define Orange_5     's'
#define Off_5        't'

#define Red_6        'v'
#define Green_6      'w'
#define Orange_6     'x'
#define Off_6        'y'

#define Test         'z'

#define All_Red      '1'
#define All_Green    '2'
#define All_Orange   '3'
#define All_Off      '4'

  void allOff()
  {
    analogWrite(RedLed_1, 0);
    analogWrite(RedLed_2, 0);
    analogWrite(RedLed_3, 0);
    analogWrite(RedLed_4, 0);
    analogWrite(RedLed_5, 0);
    analogWrite(RedLed_6, 0);

    analogWrite(GreenLed_1, 0);
    analogWrite(GreenLed_2, 0);
    analogWrite(GreenLed_3, 0);
    analogWrite(GreenLed_4, 0);
    analogWrite(GreenLed_5, 0);
    analogWrite(GreenLed_6, 0);
  }

  void allOrange()
  {
    analogWrite(RedLed_1, 255);
    analogWrite(RedLed_2, 255);
    analogWrite(RedLed_3, 255);
    analogWrite(RedLed_4, 255);
    analogWrite(RedLed_5, 255);
    analogWrite(RedLed_6, 255);

    analogWrite(GreenLed_1, 255);
    analogWrite(GreenLed_2, 255);
    analogWrite(GreenLed_3, 255);
    analogWrite(GreenLed_4, 255);
    analogWrite(GreenLed_5, 255);
    analogWrite(GreenLed_6, 255);
  }

  void allRed()
  {
    analogWrite(RedLed_1, 255);
    analogWrite(RedLed_2, 255);
    analogWrite(RedLed_3, 255);
    analogWrite(RedLed_4, 255);
    analogWrite(RedLed_5, 255);
    analogWrite(RedLed_6, 255);

    analogWrite(GreenLed_1, 0);
    analogWrite(GreenLed_2, 0);
    analogWrite(GreenLed_3, 0);
    analogWrite(GreenLed_4, 0);
    analogWrite(GreenLed_5, 0);
    analogWrite(GreenLed_6, 0);
  }

  void allGreen()
  {
    analogWrite(RedLed_1, 0);
    analogWrite(RedLed_2, 0);
    analogWrite(RedLed_3, 0);
    analogWrite(RedLed_4, 0);
    analogWrite(RedLed_5, 0);
    analogWrite(RedLed_6, 0);

    analogWrite(GreenLed_1, 255);
    analogWrite(GreenLed_2, 255);
    analogWrite(GreenLed_3, 255);
    analogWrite(GreenLed_4, 255);
    analogWrite(GreenLed_5, 255);
    analogWrite(GreenLed_6, 255);
  }
  

void setup() {
  Serial.begin(9600);

  pinMode(RedLed_1, OUTPUT);
  pinMode(RedLed_2, OUTPUT);
  pinMode(RedLed_3, OUTPUT);
  pinMode(RedLed_4, OUTPUT);
  pinMode(RedLed_5, OUTPUT);
  pinMode(RedLed_5, OUTPUT);

  pinMode(GreenLed_1, OUTPUT);
  pinMode(GreenLed_2, OUTPUT);
  pinMode(GreenLed_3, OUTPUT);
  pinMode(GreenLed_4, OUTPUT);
  pinMode(GreenLed_5, OUTPUT);
  pinMode(GreenLed_6, OUTPUT);

  allRed();
}

// the loop function runs over and over again forever
void loop()

{
  if (Serial.available())
  {
    val = Serial.read();


    // Первая табличка
    if (val == Red_1)
    {
      analogWrite(RedLed_1, 255);
      analogWrite(GreenLed_1, 0);
    }
    if (val == Green_1)
    {
      analogWrite(RedLed_1, 0);
      analogWrite(GreenLed_1, 255);
    }
    if (val == Orange_1)
    {
      analogWrite(RedLed_1, 255);
      analogWrite(GreenLed_1, pwm);
    }
    if (val == Off_1)
    {
      analogWrite(RedLed_1, 0);
      analogWrite(GreenLed_1, 0);
    }


    // Вторая табличка
    if (val == Red_2)
    {
      analogWrite(RedLed_2, 255);
      analogWrite(GreenLed_2, 0);
    }
    if (val == Green_2)
    {
      analogWrite(RedLed_2, 0);
      analogWrite(GreenLed_2, 255);
    }
    if (val == Orange_2)
    {
      analogWrite(RedLed_2, 255);
      analogWrite(GreenLed_2, pwm);
    }
    if (val == Off_2)
    {
      analogWrite(RedLed_2, 0);
      analogWrite(GreenLed_2, 0);
    }


    // Третья табличка
    if (val == Red_3)
    {
      analogWrite(RedLed_3, 255);
      analogWrite(GreenLed_3, 0);
    }
    if (val == Green_3)
    {
      analogWrite(RedLed_3, 0);
      analogWrite(GreenLed_3, 255);
    }
    if (val == Orange_3)
    {
      analogWrite(RedLed_3, 255);
      analogWrite(GreenLed_3, pwm);
    }
    if (val == Off_3)
    {
      analogWrite(RedLed_3, 0);
      analogWrite(GreenLed_3, 0);
    }


    // Четвертая табличка
    if (val == Red_4)
    {
      analogWrite(RedLed_4, 255);
      analogWrite(GreenLed_4, 0);
    }
    if (val == Green_4)
    {
      analogWrite(RedLed_4, 0);
      analogWrite(GreenLed_4, 255);
    }
    if (val == Orange_4)
    {
      analogWrite(RedLed_4, 255);
      analogWrite(GreenLed_4, pwm);
    }
    if (val == Off_4)
    {
      analogWrite(RedLed_4, 0);
      analogWrite(GreenLed_4, 0);
    }


    // Пятая табличка
    if (val == Red_5)
    {
      analogWrite(RedLed_5, 255);
      analogWrite(GreenLed_5, 0);
    }
    if (val == Green_5)
    {
      analogWrite(RedLed_5, 0);
      analogWrite(GreenLed_5, 255);
    }
    if (val == Orange_5)
    {
      analogWrite(RedLed_5, 255);
      analogWrite(GreenLed_5, pwm);
    }
    if (val == Off_5)
    {
      analogWrite(RedLed_5, 0);
      analogWrite(GreenLed_5, 0);
    }


    // Шестая табличка
    if (val == Red_6)
    {
      analogWrite(RedLed_6, 255);
      analogWrite(GreenLed_6, 0);
    }
    if (val == Green_6)
    {
      analogWrite(RedLed_6, 0);
      analogWrite(GreenLed_6, 255);
    }
    if (val == Orange_6)
    {
      analogWrite(RedLed_6, 255);
      analogWrite(GreenLed_6, pwm);
    }
    if (val == Off_6)
    {
      analogWrite(RedLed_6, 0);
      analogWrite(GreenLed_6, 0);
    }




    // Всего
    // Все красным
    if (val == All_Red)
    {
      allRed();
    }

    // Все зеленым
    if (val == All_Green)
    {
      allGreen();
    }


    // Все оранжевым
    if (val == All_Orange)
    {
      allOrange();
    }


    // Все вкл
    if (val == All_Off)
    {
      allOff();
    }


    // Тест режим
    if (val == Test)
    {
      analogWrite(RedLed_1, 1);
      analogWrite(RedLed_2, 1);
      analogWrite(RedLed_3, 0);
      analogWrite(RedLed_4, 0);
      analogWrite(RedLed_5, 1);
      analogWrite(RedLed_6, 0);

      analogWrite(GreenLed_1, 0);
      analogWrite(GreenLed_2, pwm);
      analogWrite(GreenLed_3, 1);
      analogWrite(GreenLed_4, 1);
      analogWrite(GreenLed_5, 0);
      analogWrite(GreenLed_6, 1);
    }


  }
}

















