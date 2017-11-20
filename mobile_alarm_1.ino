#include"pitches.h"
#define buzzerPin 12
#define buttonPin1 6
#define buttonPin2 7
#define ledPin 13
#define photocellPin A0
#define ambientLight 300
#define latch 10  //pin 10 STCP
#define clock 11  //pin 11 SHCP
#define data 9    //pin 9 DS

struct BUTTON {
    int state;
    int count;
    int sTime;
    int eTime;
    int duration;
    int push;
};

BUTTON button[2];

int N = 10, state = 0, light;
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
  
  NOTE_C5, NOTE_E4, NOTE_E5, NOTE_DS5,
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
  NOTE_C5, NOTE_B4, NOTE_A4, 0,
  
};

// Main tempo of 'Fur Elise'
int tempo[] = {
  9, 9, 9, 9,
  9, 9, 9, 9,
  3, 9, 9, 9,
  3, 9, 9, 9,

  3, 9, 9, 9,
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
  8, 8, 1, 9,
};

void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(photocellPin, INPUT);
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);
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
  interrupts();
  
  Serial.begin(9600);
}

//Interrupt will be called every second
ISR(TIMER1_COMPA_vect)
{
    count--;
    if (count == 0)
      state = 4;
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
                          Serial.print("Hr = ");
                          Serial.println(hr);

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
                          Serial.print("Minute = ");
                          Serial.println(minute);
                          a2 = 0;
                          break;
              case 2:     
                          Serial.println("Time is reset");
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
            Serial.print("Time is set for ");
            Serial.print(hr);
            Serial.print(" hour(s) and ");
            Serial.print(minute);
            Serial.println(" minutes");
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
              Serial.println("Alarm is disable");
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
            a2 = digitalRead(buttonPin2);
            light = analogRead(photocellPin) > ambientLight;

            if (a2 == LOW)
            {
              digitalWrite(ledPin, LOW); //Turn off the alarm sound
              Serial.println("Alarm is disable");
              for (int i = 0; i < 50; i++)
                displayClear();
              for (int i = 2; i < 6; i++)
                buff[i-2] = 17;
              TIMSK1  &= ~(1 <<  OCIE1A);  //clear interrupt for ocie1a
              hr = 0;
              minute = 0;
              state = 0;
              thisNote = 0;
            }
            else
            {
              //alarm sound
              noteDuration = 1000 / tempo[thisNote];
              tone(buzzerPin, melody[thisNote], noteDuration);
                
              delay(noteDuration * 2);   
              thisNote++;
              if (thisNote >= sizeMelody) thisNote = 0;
              
              digitalWrite(ledPin, HIGH);
              displayOff();
            }
            
            if (count == -60)
            {
              state = 5;
            }
            break;
     case 5: 
            a2 = digitalRead(buttonPin2);
            light = analogRead(photocellPin) > ambientLight;
            
            if (a2 == LOW)
            {
              noTone(buzzerPin); //Turn off the alarm sound
              Serial.println("Alarm is disable");
              TIMSK1  &= ~(1 <<  OCIE1A);  //clear interrupt for ocie1a
              hr = 0;
              minute = 0;
              state = 0;
              thisNote = 0;
            }
            else
            {
              //Alarm goes louder
              noteDuration = 1000 / tempo[thisNote];
              tone(buzzerPin, melody[thisNote], noteDuration);
                
              delay(noteDuration * 1.1);   
              thisNote++;
              if (thisNote >= sizeMelody) thisNote = 0;
            }
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

//cite this https://gist.github.com/Pietruz3000/a03f8cb4ea993a609764279e2bcbdac7

