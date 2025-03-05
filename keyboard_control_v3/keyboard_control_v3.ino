#include <Wire.h>
#define MCP4725_ADDR 0x60   


//--------------------------------------------------------------------
// BOARD
#define gate 2
#define gate_gnd 3
#define trigger 4

#define colA 5
#define colB 6
#define colC 7

#define rowA 10
#define rowB 11
#define rowC 12

// defines the voltage to send to keyboard (in) and the incoming voltage from keyboard (out)
#define in 8
#define out 9 

//--------------------------------------------------------------------
// ENCODER
#define enc_vcc 17
#define enc_sw 15

#define enc_A 16
#define enc_gnd 20
#define enc_B 21

int currEncAVal = 0;
int encCounter = 0;
int tempCorrection = 0;

int encAVal = 0;
int encBVal = 0;

//---------------------------------------------------------------------
// PITCH BEND
#define bend_down 13
#define bend_up 14

//------------------------------------------------------------------------
// SEQUENCER

//#define play 13
//#define record 14
//#define clockIn 15

#define numSteps 42

// state


// memory
int sequence[numSteps];



//------------------------------------------------------------------------
// STATE
int correction[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int correctionAmount = 1;

int valthresh = 10; // out voltage from the keyboard that triggers a button pressed event

// keyboard state.
bool buttonPressed = 0;
int currentIndex = 0; 

// scale
int scale[20];

//-------------------------------------------------------------------------
// MAIN BODY

void setup() {

  //initSequence();  
  initScale();    
  initPins();
  initBoard();  

  Wire.begin();
  //Serial.begin(9600);

  currEncAVal = digitalRead(enc_A);

  writeValueOut(scale[0] + correction[0]);
}

bool swstate = false;

void loop() { 
  
  //digitalWrite(in, HIGH);
  bool atLeastOne = 0; // no buttons pressed yet
    
  int sw = analogRead(enc_sw);
  if (sw < 100)
  {
    if (swstate == false)
    {
      //Serial.println("switch");
       correction[currentIndex] = tempCorrection;
       tempCorrection = 0;
       swstate = true;
    }
  }
  else
  {
    swstate = false;
  }

  
  //Serial.print(currentIndex); Serial.print(" "); Serial.print(tempCorrection); Serial.print(" "); Serial.println(correction[currentIndex]);

  // pitch bend
  
  int bendUp = digitalRead(bend_up);
  int bendDown = digitalRead(bend_down);
  //Serial.print(bendUp);Serial.println(bendDown);

  int maxcol = 0;
  for(int r = 5; r<8; r++)
  { 
     
    digitalWrite(rowA, (r & 1) > 0);
    digitalWrite(rowB, (r & 2) > 0);
    digitalWrite(rowC, (r & 4) > 0);
    
    

    // if r < 7, maxcol = 8, otherwise maxcol = 4
    maxcol = 8 - 4*( ((r & 4) >> 2) & ((r & 2) >> 1) & (r & 1));
    
    for (int c = 0; c < maxcol; c++)
    {
      readEncoder();
      
      digitalWrite(colA, (c & 1) > 0);
      digitalWrite(colB, (c & 2) > 0);
      digitalWrite(colC, (c & 4) > 0);

      //Serial.print((c & 4)> 0);Serial.print((c & 2)> 0);Serial.println((c & 1)> 0);
    
      delay(1); 
      
      int index = 8*(r-5) + c;     
      
      bool val = digitalRead(out);
      //Serial.print(index); Serial.print(" "); Serial.println(val);
      
      if (val > 0)
      {
        //Serial.print(index); Serial.print(" "); Serial.println(val);

        atLeastOne = 1;
              
        if (!(index & currentIndex)) // change of key actions
        {
            encCounter = 0;
            tempCorrection = 0;            
        }
        currentIndex = index;        
      }      
      else
      {
        //digitalWrite(gate, LOW);
      }
    }        
  }
 
  int valueOut = getValueIn(currentIndex);          
  writeValueOut(fullCorrectVal(valueOut));
  //writeValueOut((valueOut));


  if(!atLeastOne)
  {
    buttonPressed = 0;
    digitalWrite(gate, LOW);
  }
  else
  {
    buttonPressed = 1;    
    digitalWrite(gate, HIGH);
  }
  
 /*   
 Serial.print("current index: "); Serial.print(currentIndex); 
 Serial.print(" val: "); Serial.print(valueOut);  
 Serial.print(" correction: "); Serial.print(tempCorrection); 
 Serial.print(" counter: "); Serial.print(encCounter); 
 Serial.print(" gate: "); Serial.println(buttonPressed);  
*/
//Serial.print("correction : "); Serial.println(tempCorrection);  
}

int fullCorrectVal(int valin) 
{
  //return valin;

  int corrected = valin + correction[currentIndex] + tempCorrection;

  if (corrected < 0)
  {      
      tempCorrection = 0;
      return 0;
  }

  return corrected;
}

void readEncoder()
{
  // encoder  
  encAVal = analogRead(enc_A) > 500;  
  encBVal = analogRead(enc_B) > 500;

  if((currEncAVal == 1) && (encAVal == 0)) // a rotation has occurred
  { 
      if (encBVal == 1) // clockwise
      {
        //Serial.println("1");        
        tempCorrection += correctionAmount;
        
      }
      else
      {
        //Serial.println("-1");
        tempCorrection -= correctionAmount;
      }        
  } 
     
  currEncAVal = encAVal;    
}

int getValueIn(int index)
{
  return (scale[index]);
}

void writeValueOut(int value)
{
  Wire.beginTransmission(MCP4725_ADDR);
  Wire.write(64);          
  Wire.write(value >>4); // 8 msb
  Wire.write((value&15) <<4); //4 lsb         
  Wire.endTransmission();
}

void initSequence()
{
  for (int i = 0; i < numSteps; i++)
  {
    sequence[i] = 0;
  }  
}

void initScale()
{
  scale[0] = 0;
  scale[1] = 67 +1;
  scale[2] = 2*scale[1] +1;
  scale[3] = 3*scale[1]+2;
  scale[4] = 4*scale[1] +1;
  scale[5] = 5*scale[1] +2;
  scale[6] = 6*scale[1] +2;
  scale[7] = 7*scale[1] +1;
  scale[8] = 8*scale[1];
  scale[9] = 9*scale[1];
  scale[10] = 10*scale[1] -1;
  scale[11] = 11*scale[1] -3;
  scale[12] = 12*scale[1] -2;
  scale[13] = 13*scale[1] -2;
  scale[14] = 14*scale[1] -1;
  scale[15] = 15*scale[1] -2;
  scale[16] = 16*scale[1] -2;
  scale[17] = 17*scale[1] -2;
  scale[18] = 18*scale[1] -2;
  scale[19] = 19*scale[1] -2;
}

void initPins()
{
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(rowC, OUTPUT);
  pinMode(rowB, OUTPUT);
  pinMode(rowA, OUTPUT);

  pinMode(colC, OUTPUT);
  pinMode(colB, OUTPUT);
  pinMode(colA, OUTPUT);

  pinMode(in, OUTPUT);
  pinMode(out, INPUT);

  pinMode(gate_gnd, OUTPUT);
  pinMode(gate, OUTPUT);
  pinMode(trigger, OUTPUT);

  // encoder
  pinMode(enc_vcc, OUTPUT);
  digitalWrite(enc_vcc, HIGH);

  pinMode(enc_gnd, OUTPUT);
  digitalWrite(enc_gnd, LOW);

  pinMode(enc_sw, INPUT);
  pinMode(enc_A, INPUT);
  pinMode(enc_B, INPUT);
}
void initBoard()
{
  // initialise matrix
  digitalWrite(rowC, 0);
  digitalWrite(rowB, 0);
  digitalWrite(rowA, 0);

  digitalWrite(colC, 0);
  digitalWrite(colB, 0);
  digitalWrite(colA, 0);

  digitalWrite(gate_gnd, 0);
  digitalWrite(gate, 0);
  digitalWrite(trigger, 0);

  digitalWrite(in, HIGH);  // send 5 volts through the keyboard
}
