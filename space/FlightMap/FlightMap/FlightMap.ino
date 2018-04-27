#include <RBD_Timer.h>
#include <Bounce2.h>

#define BLINK_POINTS_LIM 3
#define SELECT_BTN 10
#define SUBMIT_BTN 11

bool blinkState;
bool finished;
RBD::Timer blinkTimer;
Bounce select = Bounce();
Bounce submit = Bounce();

int path[70][3] = {
  {18, 27, 26},  //0
  {0, 0, 0},   //1
  {8, 7, 0},   //2
  {41, 45, 0},   //3
  {41, 45, 0},   //4
  {0, 0, 0},   //5
  {0, 0, 0},   //6
  {65, 69, 0},   //7
  {0, 0, 0},   //8
  {0, 0, 0},   //9
  {0, 0, 0},   //10
  {0, 0, 0},   //11
  {0, 0, 0},   //12
  {0, 0, 0},   //13
  {0, 0, 0},   //14
  {0, 0, 0},   //15
  {0, 0, 0},   //16
  {0, 0, 0},   //17
  {20, 19, 0},   //18
  {16, 22, 0},   //19
  {0, 0, 0},   //20
  {0, 0, 0},   //21
  {0, 0, 0},   //22
  {0, 0, 0},   //23
  {0, 0, 0},   //24
  {35, 30, 0},   //25
  {33, 25, 0},    //26
  {2, 3, 0},   //27
  {0, 0, 0},   //28
  {0, 0, 0},   //29
  {0, 0, 0},   //30
  {0, 0, 0},   //31
  {0, 0, 0},   //32
  {0, 0, 0},   //33
  {0, 0, 0},   //34
  {0, 0, 0},   //35
  {0, 0, 0},   //36
  {0, 0, 0},   //37
  {0, 0, 0},   //38
  {0, 0, 0},   //39
  {0, 0, 0},   //40
  {63, 68, 0},   //41
  {0, 0, 0},   //42
  {1488, 1488, 1488},   //43 - finish
  {1488, 1488, 1488},   //44 - finish
  {0, 0, 0},   //45
  {0, 0, 0},   //46
  {0, 0, 0},   //47
  {0, 0, 0},   //48
  {43, 40, 0},   //49
  {0, 0, 0},   //50
  {0, 0, 0},   //51
  {47, 48, 0},   //52
  {0, 0, 0},   //53
  {0, 0, 0},   //54
  {0, 0, 0},   //55
  {0, 0, 0},   //56
  {0, 0, 0},   //57
  {0, 0, 0},   //58
  {0, 0, 0},   //59
  {0, 0, 0},   //60
  {0, 0, 0},   //61
  {0, 0, 0},   //62
  {0, 0, 0},   //63
  {0, 0, 0},   //64
  {0, 0, 0},   //65
  {0, 0, 0},   //66
  {0, 0, 0},   //67
  {0, 0, 0},   //68
  {49, 52, 0}    //69
};

int tail[70] = {
  0,  //0
  0,  //1
  5,  //2
  24, //3
  0,  //4
  0,  //5
  0,  //6
  9,  //7
  4,  //8
  0,  //9
  0,  //10
  0,  //11
  0,  //12
  0,  //13
  0,  //14
  0,  //15
  21, //16
  0,  //17
  17, //18
  15, //19
  14, //20
  0,  //21
  23,  //22
  22, //23
  0,  //24
  36, //25
  29, //26
  28, //27
  0,  //28
  0,  //29
  37, //30
  0,  //31
  0,  //32
  31, //33
  0,  //34
  34, //35
  0,  //36
  0,  //37
  0,  //38
  0,  //39
  32, //40
  39, //41
  0,  //42
  44, //43
  0,  //44
  42, //45
  0,  //46
  50, //47
  51, //48
  38, //49
  0,  //50
  0,  //51
  53, //52
  0,  //53
  0,  //54
  0,  //55
  0,  //56
  0,  //57
  0,  //58
  0,  //59
  0,  //60
  0,  //61
  0,  //62
  6,  //63
  0,  //64
  67, //65
  0,  //66
  0,  //67
  64, //68
  46, //69
};

void handleBlink();

void initOutputs();
void render();

int currentPoint;
int currentBlink;

void pointOn(int point)
{
  digitalWrite(point, HIGH);
  digitalWrite(tail[point], HIGH);
}

void switchBlink()
{
  digitalWrite(currentBlink, LOW);
  int * blinkPoints = path[currentPoint];
  for (int i = 0; i < BLINK_POINTS_LIM; ++i)
  {
    if (blinkPoints[i] == currentBlink)
    {
      do
      {
        i++;
        currentBlink = blinkPoints[i % 3];
      }
      while (currentBlink == 0);

      return;
    }
  }
}

void reset()
{
  finished = false;
  initOutputs();
  currentPoint = 0;
  currentBlink = 18;
}

void setup() {
  Serial.begin(9600);

  reset();

  blinkTimer.setTimeout(300);
  blinkTimer.restart();
}

long long failTime = 0;

void help()
{
  for(int i = 0; i < 70; ++i)
    digitalWrite(i, LOW);
  //27, 2, 7, 69, 49, 43
  pointOn(27);
  pointOn(2);
  pointOn(7);
  pointOn(69);
  pointOn(49);
  pointOn(43);
  delay(3000);

  failTime = 0;
  reset();
}

bool selectDown;
bool submitDown;
void loop() {

  if (Serial.available()) {
    int inByte = Serial.read();
    if (inByte == '1')
    {
      failTime = 0;
      reset();
    }
    else if (inByte == '9')
    {
      help();
    }
  }

  if (finished)
    return;

  if (currentBlink < 0)
  {
    if (failTime == 0)
    {
      failTime = millis();
    }
    else if (millis() - failTime > 3000)
    {
      failTime = 0;
      reset();
    }

    return;
  }

  handleBlink();

  select.update();
  submit.update();
  int selVal = select.read();
  int submVal = submit.read();

  if (selVal == LOW)
  {
    if (!selectDown)
    {
      selectDown = true;
      switchBlink();
    }
  }
  else
  {
    if (selectDown)
    {
      selectDown = false;
    }

  }

  if (submVal == LOW)
  {
    if (!submitDown)
    {
      submitDown = true;
      currentPoint = currentBlink;
      currentBlink = path[currentPoint][0];
      if (currentBlink == 0)
        currentBlink = -1;

      if (currentBlink == 1488)
      {
        finished = true;
        Serial.print('1');
      }

      pointOn(currentPoint);
    }
  }
  else
  {
    if (submitDown)
    {
      submitDown = false;
    }

  }
}

void handleBlink()
{
  if (blinkTimer.onRestart()) {
    if (currentBlink > 0)
    {
      blinkState = !blinkState;
      blinkState ? digitalWrite(currentBlink, HIGH) : digitalWrite(currentBlink, LOW);
    }
  }
}

void initOutputs()
{
  pinMode(SELECT_BTN, INPUT);
  pinMode(SUBMIT_BTN, INPUT);

  select.attach(SELECT_BTN);
  select.interval(10); // interval in ms
  submit.attach(SUBMIT_BTN);
  submit.interval(10); // interval in ms

  // 1
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  // 2
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(20, OUTPUT);
  pinMode(21, OUTPUT);
  // 3
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(28, OUTPUT);
  pinMode(29, OUTPUT);
  // 4
  pinMode(30, OUTPUT);
  pinMode(31, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(34, OUTPUT);
  pinMode(35, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);
  // 5
  pinMode(38, OUTPUT);
  pinMode(39, OUTPUT);
  pinMode(40, OUTPUT);
  pinMode(41, OUTPUT);
  pinMode(42, OUTPUT);
  pinMode(43, OUTPUT);
  pinMode(44, OUTPUT);
  pinMode(45, OUTPUT);
  // 6
  pinMode(46, OUTPUT);
  pinMode(47, OUTPUT);
  pinMode(48, OUTPUT);
  pinMode(49, OUTPUT);
  pinMode(50, OUTPUT);
  pinMode(51, OUTPUT);
  pinMode(52, OUTPUT);
  pinMode(53, OUTPUT);


  pinMode(63, OUTPUT);
  pinMode(64, OUTPUT);
  pinMode(65, OUTPUT);
  pinMode(67, OUTPUT);
  pinMode(68, OUTPUT);
  pinMode(69, OUTPUT);

  for (int i = 2; i < 70; ++i)
  {
    digitalWrite(i, LOW);
  }
}


