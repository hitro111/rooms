#define SOUND

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BACKLIGHT_PIN 9

const int ConnectPin = A0;
int buz = 3;
int a = 5;
int b = 10;
LiquidCrystal_I2C lcd(0x27, 16, 2);   /* Задаем адрес и размерность дисплея.
  При использовании LCD I2C модуля с дисплеем 20х04 ничего в коде изменять не требуется, cледует только задать правильную размерность */

volatile bool interrupted = false;
volatile bool processingStarted = false;

void doInterrupt()
{
  processingStarted = false;
  interrupted = true;
}

void setup()
{
  //Serial.begin(9600);
  randomSeed(analogRead(0));
  pinMode(ConnectPin, INPUT);

  pinMode(BACKLIGHT_PIN, OUTPUT);
  analogWrite(BACKLIGHT_PIN, 60);

  pinMode(buz, OUTPUT);
  lcd.init();                       // Инициализация lcd
  lcd.backlight();                  // Включаем подсветку
  lcd.clear();
  delay (500);
  loading();
  delay (500);
  lcd.clear();
  delay (300);
  done();
  delay (100);
  done_sound();

  //    \xf4 Значок омм
  //    \xf7 Корявая Л
  //    \xf9 Корявая Ч
  //    \xae мелкая з
  //    \xdb прямоугольник
  //    \xdf значок градуса
  //    \xff заполненый прямоугольник


}

void handleInterrupted()
{
#ifdef SOUND
  noTone(buz);
#endif
  lcd.clear();
}

void checkInterrupt()
{
  if (analogRead(ConnectPin) > 600)
  {
    doInterrupt();
  }
}

#define MAX_CODE_SIZE 6
int CODE_SIZE = 6;
unsigned long long grab_time = 12000;
int code[MAX_CODE_SIZE] = {6, 3, 7, 8, 2, 2};
int seq[MAX_CODE_SIZE];

void loop()
{
  int val = analogRead(ConnectPin);
  if (val < 600 && !processingStarted)
  {
    processingStarted = true;
    interrupted = false;
    lcd.clear();

    checkInterrupt();
    if (interrupted)
    {
      handleInterrupted();
      return;
    }

    connection();
    lcd.clear();

    checkInterrupt();
    if (interrupted)
    {
      handleInterrupted();
      return;
    }
    delay (500);

    if (val < 400) //325
    {
      CODE_SIZE = 6;
      grab_time = 12000;
      code[0] = 6;
      code[1] = 3;
      code[2] = 7;
      code[3] = 8;
      code[4] = 2;
      code[5] = 2;
    }
    else //510
    {
      CODE_SIZE = 4;
      grab_time = 8000;
      code[0] = 2;
      code[1] = 7;
      code[2] = 8;
      code[3] = 4;
    }

    selection1();

    checkInterrupt();
    if (interrupted)
    {
      handleInterrupted();
      return;
    }
    delay (500);
    success();
  }

  checkInterrupt();
  if (interrupted)
  {
    handleInterrupted();
    return;
  }

  delay(10);



}

void done()  // Готово
{

  byte let_G[8] =
  {
    B11111,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B00000,
  };
  byte let_t[8] =
  {
    B00000,
    B00000,
    B11111,
    B00100,
    B00100,
    B00100,
    B00100,
    B00000,
  };
  byte let_v[8] =
  {
    B00000,
    B00000,
    B11110,
    B10001,
    B11110,
    B10001,
    B11110,
    B00000,
  };
  lcd.createChar(1, let_G);
  lcd.createChar(2, let_t);
  lcd.createChar(3, let_v);
  lcd.print("\1o\2o\3o");
}

void done_sound()  // Готово
{
#ifdef SOUND
  tone(buz, 800);
  delay(50);
  tone(buz, 1500);
  delay(50);
  tone(buz, 800);
  delay(50);
  tone(buz, 1500);
  delay(50);

  tone(buz, 800);
  delay(50);
  tone(buz, 1500);
  delay(50);
  tone(buz, 800);
  delay(50);
  tone(buz, 1500);
  delay(50);


  tone(buz, 5000);
  delay(150);
  noTone(buz);
#endif
}









void loading()  // Загрузка устройства
{

  byte let_Z[8] =
  {
    B11100,
    B00010,
    B00010,
    B11110,
    B00001,
    B00001,
    B11110,
    B00000,
  };
  byte let_g[8] =
  {
    B00000,
    B00000,
    B11111,
    B10000,
    B10000,
    B10000,
    B10000,
    B00000,
  };
  byte let_u[8] =
  {
    B00000,
    B00000,
    B10001,
    B10001,
    B01111,
    B00001,
    B11110,
    B00000,
  };
  byte let_z[8] =
  {
    B00000,
    B00000,
    B11110,
    B00001,
    B00110,
    B00001,
    B11110,
    B00000,
  };
  byte let_k[8] =
  {
    B00000,
    B00000,
    B10001,
    B10010,
    B11100,
    B10010,
    B10001,
    B00000,
  };

  lcd.createChar(1, let_Z);
  lcd.createChar(2, let_g);
  lcd.createChar(3, let_u);
  lcd.createChar(4, let_z);
  lcd.createChar(5, let_k);
  lcd.setCursor(0, 0);
  lcd.print("\1a\2p\3\4\5a");                     // Выводим смайлик (символ под номером 1) - "\1"
  delay (500);


  for (int i = 0; i < 16; i++)
  {
#ifdef SOUND
    tone(buz, 1);
    delay(5);
    noTone(buz);
#endif
    lcd.setCursor(i, 1);
    lcd.print("\xff");
    delay(1000);
  }

}





void success()  // Код определен
{

  byte let_d[8] =
  {
    B00000,
    B00000,
    B01111,
    B00101,
    B01001,
    B10001,
    B11111,
    B10001,
  };
  byte let_p[8] =
  {
    B00000,
    B00000,
    B11111,
    B10001,
    B10001,
    B10001,
    B10001,
    B00000,
  };
  byte let_l[8] =
  {
    B00000,
    B00000,
    B11111,
    B01001,
    B01001,
    B01001,
    B10001,
    B00000,
  };


  byte let_n[8] =
  {
    B00000,
    B00000,
    B10001,
    B10001,
    B11111,
    B10001,
    B10001,
    B00000,
  };
  byte let_e[8] =
  {
    B00000,
    B00000,
    B10001,
    B10001,
    B10011,
    B10101,
    B11001,
    B00000,
  };


  lcd.createChar(1, let_d);
  lcd.createChar(2, let_p);
  lcd.createChar(3, let_l);
  lcd.createChar(4, let_n);

  lcd.setCursor(0, 0);
  lcd.print("               ");
  delay (2000);
#ifdef SOUND
  tone(buz, 2000);
#endif
  lcd.setCursor(0, 0);
  lcd.print("Ko\1 o\2pe\1e\3e\4");                     // Выводим смайлик (символ под номером 1) - "\1"
  delay (1000);
#ifdef SOUND
  noTone(buz);
#endif
  lcd.setCursor(0, 0);
  lcd.print("               ");
  delay (1000);
#ifdef SOUND
  tone(buz, 2000);
#endif
  lcd.setCursor(0, 0);
  lcd.print("Ko\1 o\2pe\1e\3e\4");                      // Выводим смайлик (символ под номером 1) - "\1"
  delay (1000);
#ifdef SOUND
  noTone(buz);
#endif
  lcd.setCursor(0, 0);
  lcd.print("               ");
  delay (1000);
#ifdef SOUND
  tone(buz, 2000);
#endif
  lcd.setCursor(0, 0);
  lcd.print("Ko\1 o\2pe\1e\3e\4");                      // Выводим смайлик (символ под номером 1) - "\1"
  delay (1000);
#ifdef SOUND
  noTone(buz);
#endif


}




















void connection()  // Подключен
{
  checkInterrupt();
  if (interrupted)
  {
    handleInterrupted();
    return;
  }

  byte let_P[8] =
  {
    B11111,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
  };
  byte let_d[8] =
  {
    B00000,
    B00000,
    B01111,
    B00101,
    B01001,
    B10001,
    B11111,
    B10001,
  };
  byte let_k[8] =
  {
    B00000,
    B00000,
    B10001,
    B10010,
    B11100,
    B10010,
    B10001,
    B00000,
  };
  byte let_l[8] =
  {
    B00000,
    B00000,
    B11111,
    B01001,
    B01001,
    B01001,
    B10001,
    B00000,
  };
  byte let_u[8] =
  {
    B00000,
    B00000,
    B10111,
    B10101,
    B11101,
    B10101,
    B10111,
    B00000,
  };
  byte let_ch[8] =
  {
    B00000,
    B00000,
    B10001,
    B10001,
    B10001,
    B11111,
    B00001,
    B00000,
  };
  byte let_n[8] =
  {
    B00000,
    B00000,
    B10001,
    B10001,
    B11111,
    B10001,
    B10001,
    B00000,
  };
  byte let_e[8] =
  {
    B00000,
    B00000,
    B10001,
    B10001,
    B10011,
    B10101,
    B11001,
    B00000,
  };

  lcd.createChar(1, let_P);
  lcd.createChar(2, let_d);
  lcd.createChar(3, let_k);
  //  lcd.createChar(4, let_l);
  lcd.createChar(4, let_u);
  lcd.createChar(5, let_ch);
  lcd.createChar(6, let_n);
  lcd.createChar(7, let_e);


  lcd.setCursor(0, 0);
  lcd.print("\1o\2\3\xf7\4\5e\6\7e");                     // Выводим смайлик (символ под номером 1) - "\1"
  delay (1000);
  for (int i = 0; i < 16; i++)
  {
    checkInterrupt();
    if (interrupted)
    {
      handleInterrupted();
      return;
    }
#ifdef SOUND
    tone(buz, 1);
    delay(5);
    noTone(buz);
#endif
    lcd.setCursor(i, 1);
    lcd.print("\xff");
    delay(200);
  }
}

void selection1()  // Подбор Кода
{
  byte let_P[8] =
  {
    B11111,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
  };
  byte let_d[8] =
  {
    B00000,
    B00000,
    B01111,
    B00101,
    B01001,
    B10001,
    B11111,
    B10001,
  };
  byte let_b[8] =
  {
    B00011,
    B01100,
    B10000,
    B11110,
    B10001,
    B10001,
    B01110,
    B00000,
  };

  byte let_k[8] =
  {
    B00000,
    B00000,
    B10001,
    B10010,
    B11100,
    B10010,
    B10001,
    B00000,
  };

  lcd.createChar(1, let_P);
  lcd.createChar(2, let_d);
  lcd.createChar(3, let_b);
  lcd.createChar(4, let_k);


  lcd.setCursor(0, 0);
  lcd.print("\1o\2\3op \4o\2a");                     // Выводим смайлик (символ под номером 1) - "\1"
  delay (1000);

  for (int i = 0; i < CODE_SIZE; ++i)
  {
    while (true)
    {
      checkInterrupt();
      if (interrupted)
      {
        handleInterrupted();
        return;
      }

      seq[i] = random(0, CODE_SIZE);

      bool duplicate = false;
      for (int j = i - 1; j >= 0; --j)
      {
        if (seq[i] == seq[j])
        {
          duplicate = true;
          break;
        }
      }

      if (!duplicate)
      {
        break;
      }
    }
  }

  unsigned long long searchInterval = grab_time / CODE_SIZE;
  unsigned long long started = millis();
  unsigned long long lastFound = millis();
  int curSeq = -1;

  while (millis() - started < grab_time + 200)
  {
    checkInterrupt();
    if (interrupted)
    {
      handleInterrupted();
      return;
    }

    for (int i = 0; i < CODE_SIZE; ++i)
    {
      if (millis() - lastFound > searchInterval)
      {
        curSeq++;
        lastFound = millis();
      }

      lcd.setCursor(seq[i], 1);
      if (i > curSeq)
      {
        lcd.print(random(0, 10));
      }
      else
      {
        lcd.print(code[seq[i]]);
      }
#ifdef SOUND
      delay(5);
      tone(buz, a);
      delay(5);
      tone(buz, b);
#endif
    }
  }
#ifdef SOUND
  noTone(buz);
#endif
}
