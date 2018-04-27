#define ledPin 13
#define limit 100
#define SIZE 750
#define ERR 4

volatile bool ar[SIZE] = {0};
volatile int curSize = 0;
volatile int i;
volatile bool rdy = false;

void setup()
{

  // initialize timer1
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 250;            // compare match register 16MHz/256/2Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12);    // 256 prescaler
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts
  pinMode(ledPin, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(13, OUTPUT);

  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(13, LOW);
}

volatile int val;

ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
  if (curSize == SIZE - 1)
  {
    rdy = true;
    for (i = 0; i < SIZE; ++i)
      ar[i] = ar[i + 1];
    curSize--;
  }

  val = analogRead(A5);
  ar[curSize] = val > limit ? true : false;
  
  curSize++;
}

int j;
bool start;
int ct, ct_f, buf, buf_f;
int changeCt;
void loop()
{
  if (!rdy)
    return;

  start = false;
  ct = 0;
  buf = 0;
  changeCt = 0;
  ct_f = 0;
  buf_f = 0;

  for (j = 0; j < SIZE - 1; ++j)
  {
    //Serial.print(ar[j]);
    //Serial.print(",");

    if (!start && ar[j] != ar[j + 1])
      start = true;

    if (start)
    {
      if (ar[j] == ar[j - 1])
      {
        ar[j] ? ct++ : ct_f++;
      }
      else
      {
        changeCt++;

        if (ar[j - 1])
        {
          if (buf > 0 && abs(buf - ct) > ERR)
            return; //ERROR SIGNAL
          buf = ct;
          ct = 0;
        }
        else
        {
          if (buf_f > 0 && abs(buf_f - ct_f) > ERR)
            return; //ERROR SIGNAL

          buf_f = ct_f;
          ct_f = 0;
        }
      }
    }
  }

  if (start && changeCt > 9)
  {
    digitalWrite(6, HIGH);
    digitalWrite(7, HIGH);
    digitalWrite(13, HIGH);
    delay(3000);
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
    digitalWrite(13, LOW);
  }

}

