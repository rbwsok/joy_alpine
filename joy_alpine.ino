/////////////////////////////////////////////////////////////////
// alpine
#define alpinePin 12

// alpine commands:
#define cmdPower      0x09
#define cmdSource     0x0A
#define cmdBand       0x0D
#define cmdVolumeDown 0x15
#define cmdVolumeUp   0x14
#define cmdUp         0x0E
#define cmdDown       0x0F
#define cmdLeft       0x13
#define cmdRight      0x12
#define cmdPlay       0x07
#define cmdAProc      0x06
#define cmdMute       0x16
#define cmdAngleUp    0x4C
#define cmdAngleDown  0x4B
#define cmdCDChange   0x03
#define cmdPhoneOn    0x56
#define cmdPhoneOff   0x5C

/////////////////////////////////////////////////////////////////
// joystick
#define rows 3
#define cols 2
// keys number (from https://e-a.d-cd.net/f4f8accs-960.jpg)
const byte keys[rows][cols] = 
{
  {1,4},
  {2,5},
  {6,3}
};

// encoder direction
const byte encoderdirection[3][3] = // 0 - none, 1 - down, 2 - up
{
  {0, 1, 2},  // 0
  {2, 0, 1},  // 1
  {1, 2, 0}   // 2     
};

// 10 9 8 7 6 5

// arduino nano pins
const byte rowPins[rows] = {10, 9, 8}; // rows (yellow, green, blue)
const byte colPins[cols] = {7, 5}; // columns (black, red)
#define encoderPin 6 // encoder (brown)
byte encodervalue;

// keys masks
#define MaskMediaButton      1 
#define MaskRadioButton      2
#define MaskVolumeUpButton   4
#define MaskVolumeDownButton 8
#define MaskOkButton         16
#define MaskMuteButton       32 
#define MaskScrollDown       64 
#define MaskScrollUp         128 

unsigned long starttime;
bool workkbd;

/////////////////////////////////////////////////////////////////

void setup() 
{  
  int i;
  // rows - output
  for (i = 0; i < rows; ++i)
  {
    pinMode(rowPins[i], OUTPUT);
  }
  
  // columns - input
  for (i = 0; i < cols; ++i)
  {
    pinMode(colPins[i], INPUT);
    digitalWrite(colPins[i], HIGH); // pullup resistor on
  }

  // encoder
  pinMode(encoderPin, INPUT);
  digitalWrite(encoderPin, HIGH); // pullup resistor on
   
  encodervalue = 0;
  
  // Alpine control
  pinMode(alpinePin, OUTPUT);
  
  starttime = millis();
  workkbd = false;
}

// joystick scan
byte scan(void)
{
  int i, j;
  byte code = 0;
          
  for (i = 0; i < rows; ++i)
  {
    digitalWrite(rowPins[i], LOW);

    // buttons
    for (j = 0; j < cols; ++j)
    {
      if(digitalRead(colPins[j]) == false)
      {
        code |= 1 << (keys[i][j] - 1);
      }         
    }
       
    // encoder
    if (digitalRead(encoderPin) == false)
    {
      int oldencodervalue = encodervalue;
      encodervalue = i;
                        
      switch (encoderdirection[oldencodervalue][encodervalue])
      {
        case 1:  // вниз
          code |= 64;
          break;
        case 2:  // вверх
          code |= 128;
          break;
      }
    }
        
    digitalWrite(rowPins[i], HIGH);    
  }
  
  return code;
}

void SendByte(byte data)
{
  for (int i = 0; i < 8; ++i)
  {
    digitalWrite(alpinePin, HIGH); 
    delayMicroseconds(560);   
    digitalWrite(alpinePin, LOW); 

    if (data & 1)
    { // 1
      delayMicroseconds(1680);
    }
    else
    { // 0
      delayMicroseconds(560);
    }
    
    data >>= 1;      
  }
}

// send command
void Send(byte command) 
{ 
  digitalWrite(alpinePin, HIGH); 
  delayMicroseconds(9000); 
  digitalWrite(alpinePin, LOW); 
  delayMicroseconds(4500); 

  SendByte(0x86);
  SendByte(0x72);
  SendByte(command);
  SendByte(~command);  

  digitalWrite(alpinePin, HIGH); 
  delayMicroseconds(560); 
  digitalWrite(alpinePin, LOW); 
}

// loop cycle
void loop(void) 
{
  byte code; 
    
  code = scan();
  
  if (workkbd == false)
  {
    if (millis() - starttime > 3000)
      workkbd = true;
    
    return;
  }

  if (code & MaskMediaButton)
    Send(cmdDown);

  if (code & MaskRadioButton)
    Send(cmdUp);

  if (code & MaskVolumeUpButton)
    Send(cmdRight);

  if (code & MaskVolumeDownButton)
    Send(cmdLeft);

  if (code & MaskOkButton)
    Send(cmdPlay);

  if (code & MaskMuteButton)  
    Send(cmdMute);

  if (code & MaskScrollDown)
    Send(cmdVolumeDown);

  if (code & MaskScrollUp)
    Send(cmdVolumeUp);
}


