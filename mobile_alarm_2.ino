#include "pitches.h"

#define buzzerPin 52
#define buttonPin1 40
#define buttonPin2 44
#define disalarmPin 20
#define photocellPin A15
#define ambientLight 300
#define latch 32 //pin 12 STCP 10
#define clock 36 //pin 11 SHCP 11
#define data 28  //pin 14 DS  9

#define TRIGGER_PIN 47
#define ECHO_PIN 49

#define motorPin1 6
#define motorPin2 7
#define motorPin3 10
#define motorPin4 9

#define STOP_TIMING 1000
#define DATA_LENGTH 20

long int sTime, eTime;
int dist[54];

struct BUTTON {
    int state;
    int count;
    int sTime;
    int eTime;
    int duration;
    int push;
};

BUTTON button[2];

int state = 0;
int N = 10, light;
int hr, minute, count = 0, flag;
int buff[4] = {17, 17, 17, 17};
unsigned char table[]=
{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f, //0 to 9
  0x6d, 0x79, 0x78, 0x00,//SET
  0x39, 0x38, 0x50, 0x00,  //CLR
  0x80
};
int sizeMelody;
int thisNote, noteDuration;

// Fur Elise --> Melody
int melody[] = {
  NOTE_E5, NOTE_DS5, NOTE_E5, NOTE_DS5,
  NOTE_E5, NOTE_B4, NOTE_D5, NOTE_C5,
  NOTE_A4, NOTE_C4, NOTE_E4, NOTE_A4,
  NOTE_B4, NOTE_E4, NOTE_GS4, NOTE_B4,
  
  /*NOTE_C5, NOTE_E4, NOTE_E5, NOTE_DS5,
  NOTE_E5, NOTE_DS5, NOTE_E5, NOTE_B4,
  NOTE_D5, NOTE_C5, NOTE_A4, NOTE_C4,
  NOTE_E4, NOTE_A4, NOTE_B4, NOTE_E4,
  
  NOTE_C5, NOTE_B4, NOTE_A4, NOTE_B4,
  NOTE_C5, NOTE_D5, NOTE_E5, NOTE_G4,
  NOTE_F5, NOTE_E5, NOTE_D5, NOTE_F4,
  NOTE_E5, NOTE_D5, NOTE_C5, NOTE_E4,
  
  NOTE_D5, NOTE_C5, NOTE_B4, NOTE_E4,
  NOTE_E5, NOTE_E4, NOTE_E5, NOTE_E4,
  NOTE_E5, NOTE_E4, NOTE_E5, NOTE_DS4,
  NOTE_E5, NOTE_D4, NOTE_E5, NOTE_DS4,
  
  NOTE_E5, NOTE_B4, NOTE_D5, NOTE_C5,
  NOTE_A4, NOTE_C4, NOTE_E4, NOTE_A4,
  NOTE_B4, NOTE_E4, NOTE_GS4, NOTE_B4,
  NOTE_C5, NOTE_E4, NOTE_E5, NOTE_DS5,

  NOTE_E5, NOTE_DS5, NOTE_E5, NOTE_B4,
  NOTE_D5, NOTE_C5, NOTE_A4, NOTE_C4,
  NOTE_E4, NOTE_A4, NOTE_B4, NOTE_E4,
  NOTE_C5, NOTE_B4, NOTE_A4, */0
  
};

// Main tempo of 'Fur Elise'
int tempo[] = {
  9, 9, 9, 9,
  9, 9, 9, 9,
  3, 9, 9, 9,
  3, 9, 9, 9,

  /*3, 9, 9, 9,
  9, 9, 9, 9,
  9, 9, 3, 9,
  9, 9, 3, 9,

  9, 9, 3, 9,
  9, 9, 3, 9,
  9, 9, 3, 9,
  9, 9, 3, 9,

  9, 9, 9, 9,
  9, 9, 9, 9,
  9, 9, 9, 9,
  9, 9, 9, 9,

  9, 9, 9, 9,
  3, 9, 9, 9,
  3, 9, 9, 9,
  3, 9, 9, 9,

  9, 9, 9, 9,
  9, 9, 3, 9,
  9, 9, 3, 8,
  8, 8, 1,*/ 9
};

void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(photocellPin, INPUT);
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  pinMode(disalarmPin, INPUT_PULLUP);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  for (int i = 2; i < 6; i++)
    pinMode(i, OUTPUT);
  sizeMelody = sizeof(melody) / sizeof(int);
  //initialize timer1
  noInterrupts();
  TCCR1A  = 0;
  TCCR1B  = 0;
  TCNT1   = 0;

  OCR1A = 62500;
  TCCR1B  |= (1 <<  WGM12);   //CTC mode with top = ocr1a
  TCCR1B  |= (0x4 <<  CS10); //prescaler 

  attachInterrupt(digitalPinToInterrupt(disalarmPin), Disalarm, LOW);
  
  interrupts();
  
}

//Interrupt will be called every second
ISR(TIMER1_COMPA_vect)
{
    count--;
    if (count == 0)
      state = 4;
}

void Disalarm()
{
    light = analogRead(photocellPin) > ambientLight;

    if (light)
    {
              halt();
              for (int i = 0; i < 50; i++)
                displayClear();
              for (int i = 2; i < 6; i++)
                buff[i-2] = 17;
              TIMSK1  &= ~(1 <<  OCIE1A);  //clear interrupt for ocie1a
              hr = 0;
              minute = 0;
              state = 0;
              displayOff();
              
    }
}

void loop() {
  static int a1, a2;
  switch (state)
  {
    case 0: //Idle state, waiting for the alarm to be set          
            a1 = buttonPress(buttonPin1);
            a2 = buttonPress(buttonPin2);
            if (a1 || a2)
              state = 1;
            Display(17);
            halt();
            break;
    case 1:
            displayTimer();
            convert();
            switch (a1)
            {
              case 0:
                          a1 = buttonPress(buttonPin1);
                          break;
              case 1:     
                          hr += 1;
                          if (hr == 12) hr = 0;
                          a1 = 0;
                          break;
              case 2:     state = 2;
                          a1 = 0;
                          break;     
            }
            
            switch (a2)
            {
              case 0:
                          a2 = buttonPress(buttonPin2);
                          break;
              case 1:     
                          minute += 1;
                          if (minute == 60) minute = 0;
                          a2 = 0;
                          break;
              case 2:     
                          //Serial.println("Time is reset");
                          for (int i = 0; i < 50; i++)
                            displayClear();
                          for (int i = 2; i < 6; i++)
                            buff[i-2] = 17;
                          hr = 0;
                          minute = 0;
                          state = 0;
                          break;
            }
            break;
    case 2:
            displayTimer();
            for (int i = 0; i < 50; i++)
              displaySet();
            count = 60 * (hr * 60 + minute);    
            TIMSK1  |= (1 <<  OCIE1A);  //set interrupt for ocie1a
            state = 3;
            break;
     case 3:
            //wait for the time is up
            a2 = buttonPress(buttonPin2);
            if (a2 == 2)
            {
              for (int i = 0; i < 50; i++)
                displayClear();
              for (int i = 2; i < 6; i++)
                buff[i-2] = 17;
              TIMSK1  &= ~(1 <<  OCIE1A);  //clear interrupt for ocie1a
              hr = 0;
              minute = 0;
              state = 0;
            }
            else if (a2 == 1)
            {
              //show current timer
              hr = (count / 3600);
              minute = (count % 3600) / 60;
              convert();
              for (int i = 0; i < 50; i++)
                displayTimer();
            }
            else
              displayOn();
            break;
     case 4:    
              //alarm sound
              halt();
              
              for (thisNote = 0; thisNote < sizeMelody; thisNote++)
              {
                noteDuration = 1000 / tempo[thisNote];
                tone(buzzerPin, melody[thisNote], noteDuration);
                delay(noteDuration * 1.9);   
              }

              noTone(buzzerPin);

              //run
              Motor_Operate();

              
            if (count <= -60)
            {
              state = 5;
            }
            break;
     case 5: 
            
              //Alarm goes louder
              halt();
              
              for (thisNote = 0; thisNote < sizeMelody; thisNote++)
              {
                noteDuration = 1000 / tempo[thisNote];
                tone(buzzerPin, melody[thisNote], noteDuration);
                delay(noteDuration * 1.1);   
              }
  
              noTone(buzzerPin);

              Motor_Operate();

            break;
  }
}

int buttonPress(int pin) {

  int index;
  
  if (pin == buttonPin1) index = 0;
  else if (pin == buttonPin2) index = 1;

  button[index].push = digitalRead(pin);
  
  switch (button[index].state)
  {
    case 0: 
            if (button[index].push == LOW) 
            {
              button[index].sTime = millis();
              button[index].state = 1;
            }
            button[index].count = 0;
            break;
    case 1: 
            if (button[index].push == LOW) button[index].count++;
            else  button[index].count = 0;
            if (button[index].count >= N)
            {
              button[index].state = 2;
              button[index].count = 0;
            }
            break;
    case 2: 
            if (button[index].push == HIGH)
            {
              button[index].eTime = millis();
              button[index].count++;
            }
            else button[index].count = 0;
            if (button[index].count >= N)
              button[index].state = 3;
            break;
    case 3:
            button[index].duration = button[index].eTime - button[index].sTime;
            button[index].state = 0;
            if (button[index].duration < 500)
              return 1;
            else
              return 2;
            break;    
  }

  return 0;
}

void Display(unsigned char num)
{
  digitalWrite(latch, LOW);
  shiftOut(data, clock, MSBFIRST, table[num]);
  digitalWrite(latch, HIGH);
}

void displayTimer(void)
{
  for (int i = 2; i < 6; i++)
  {
    Display(buff[i-2]);
    digitalWrite(i, LOW);
    delayMicroseconds(2000);
    digitalWrite(i, HIGH);
  }
}

void displayClear(void)
{
  int j = 14;
  for (int i = 2; i < 6; i++)
  {
    Display(j++);
    digitalWrite(i, LOW);
    delayMicroseconds(2000);
    digitalWrite(i, HIGH);
  }
}

void displaySet(void)
{
  int j = 10;
  for (int i = 2; i < 6; i++)
  {
    Display(j++);
    digitalWrite(i, LOW);
    delayMicroseconds(2000);
    digitalWrite(i, HIGH);
  }
}

void displayOn(void)
{
  digitalWrite(3, LOW);
  Display(18);
}

void displayOff(void)
{
  digitalWrite(3, HIGH);
}

void convert(void)
{
  buff[0] = ((hr / 10) == 0)? 17 : hr / 10;
  buff[1] = hr % 10;
  buff[2] = ((minute / 10) == 0)? 17 : minute / 10;
  buff[3] = minute %10;
}

void Motor_Operate() {
  int i, maxValue, maxPos, newDist;

  if (state)
  {
    sTime = eTime = millis();
    while (eTime - sTime <= STOP_TIMING)
    {
      dist[i] = Distance();
      i++;
      turnleft();
      delay(50);
      eTime = millis();
    }
  }
  
  halt();
  delay(500);
  
  maxValue = dist[0];
  maxPos = 0;
  
  for (i = 1; i < DATA_LENGTH; i++)
  {
    if (dist[i] >= maxValue) {maxValue = dist[i]; maxPos = i;}
  }

  if (state)
  {
    newDist = Distance();
    while (newDist < maxValue * 0.8 || newDist > maxValue*1.2)
    {
      turnright();
      delay(50);
      newDist = Distance();
    }
  }
  
  halt();
  delay(500);

  if (state)
   while (Distance() > 5)  
    forward();

  if (state)
    backward();
  
  delay(700);
}

int Distance()
{
  int duration = 0;
  
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(0);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH, 6000);

  if (duration == 0) 
    return 100;
    
  return duration / 58.82;
}

void forward() 
{
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, HIGH);
}

void backward()
{
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, HIGH);
    digitalWrite(motorPin4, LOW);
  
}


void turnleft()
{
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, HIGH);
   
}

void turnright()
{
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
    
}

void halt()

{
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
 
}
//cite this https://gist.github.com/Pietruz3000/a03f8cb4ea993a609764279e2bcbdac7

