/*
 * Classic game of Snek on 8x8 Red-Green Dot Matrix
 * uses:
 *        3x 74hc595 - one for -red, one -green, one for +common or +plus
 *        3x SPST button  - left, right and reset
 *        8x 220 Ohm resistors (+ optional power-wasting resistor for powerbank)
 *
 *
 *  TODO:
 *        BETTER DOCUMENTATION
 *        INPUT PULLUP BUTTONS
 *        KICAD SCHEMATIC
 *        rewrite boolToShort
 *
 *  Snek is red,
 *  food is green
 *  Eat food to grow
 *  Don't crash in the wall or yourself
 *
 *  pins:
 *      D2 - RST              for all registers
 *      D3 - DataIn           for all registers
 *      D4 - SCLK+            for "Plus" register
 *      D5 - LCLK+            for "Plus" register
 *      D6 - SCLKr and LCLKr  for "Red" register
 *      D7 - SCLKg and LCLKg  for "Green" register
 *      D8 - button           turns Snek left
 *      D9 - button           turns Snek right
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#define FA false
#define T true

short reset = 2;    //common for all 3 registers
short data = 3;     //common for all 3 registers
short plusSclk = 4;
short plusLclk = 5;
short redClk = 6;   //red register has shift and latch clock tied together
short greenClk = 7; //green register has shift and latch clock tied together
short rightPin = 8; //turns Snek left
short leftPin = 9;  //turn Snek right
long turns=0;
short nowRow=0,nowCol=0;
int oneStep = 500;
long long curTime = 0;

short snek[64]={0,0,0,0,0,0,0,0,0,0,0,0,0}; //position of snek blocks
short collectible=-1; //position of collectible
short starts[16]={18, 19, 20, 21, 26, 27, 28, 29, 34, 35, 36, 37, 43, 44, 45, 46};  //possible starting locations for Snek
short smer = 0;     //Snek direction
short snekLength = 2;
short toLenghten = 0;
bool snekBool[8][8]; // true == snek present?
bool goLeft = false;
bool leftLatch = false;
bool goRight = false;
bool rightLatch = true;
short currDisplay[8];


   //********functions
void ResetAll()
{
  digitalWrite(reset, LOW);
  digitalWrite(reset, HIGH);
}
void drawDot(int ourDot)
{
  sendGreen(0);
  digitalWrite(plusLclk, LOW);

  short runner;
  for(runner = 0; runner < ourDot/8+1; runner++)
  {
    sendGreen(1);
  }

  shiftOut(data, plusSclk, LSBFIRST, 1<<((ourDot%8)));
  digitalWrite(plusLclk, HIGH);
  delay(2);
  shiftOut(data, plusSclk, LSBFIRST, 0);
  digitalWrite(plusLclk,LOW);
  digitalWrite(plusLclk, HIGH);
  while(runner < 10)
  {
    sendGreen(1);
    runner++;
  }


}

void makeCollectible()
{
  bool flag = true;
  int newPos;
  while(flag)
  {
    newPos = random(0,63);
    flag = snekBool[newPos/8][newPos%8]; //if the new collectifle is inside the Snek
  }
  collectible = newPos;
  Serial.print("collectible ");
  Serial.println(collectible);
}


void boolToShort()  //upgrade to shiftout?
{

  for(int x =0; x < 8; x++)
  {
    int temp = 128;
    int output= 0;
    for(int y = 0; y < 8; y++)
    {
      if(snekBool[x][y])
      {
        output = output+temp;
      }
      temp = temp/2;

    }
    currDisplay[x]=output;
  }
}
inline void sendPlusSclk(int kek)
{
  if(kek)
  {
    digitalWrite(data, HIGH);
  }
  else
  {
    digitalWrite(data, LOW);
  }
  digitalWrite(plusSclk, LOW);
  digitalWrite(plusSclk, HIGH);
  digitalWrite(data,LOW);
}
inline void sendPlusLclk()
{
  digitalWrite(plusLclk, HIGH);
  digitalWrite(plusLclk, LOW);
  digitalWrite(plusLclk, HIGH);
}

void displayStuff()
{
  sendRed(0);
  for(int x = 0; x < 8; x++)
  {
    digitalWrite(plusLclk, LOW);
    shiftOut(data, plusSclk, MSBFIRST, currDisplay[x]);
    digitalWrite(plusLclk, HIGH);
    sendRed(1);
    delay(2);
  //delay(2);
  }
  shiftOut(data, plusSclk, MSBFIRST, 0);
  digitalWrite(plusLclk, LOW);
  digitalWrite(plusLclk, HIGH);
  sendRed(1);

  drawDot(collectible);
  shiftOut(data, plusSclk, MSBFIRST, 0);
  digitalWrite(plusLclk, LOW);
  digitalWrite(plusLclk, HIGH);


}

void sendGreen(int kek)
{

  if(kek > 0)
  {
    digitalWrite(data, HIGH);
  }
  else
  {
    digitalWrite(data, LOW);
  }
  digitalWrite(greenClk, LOW);
  digitalWrite(greenClk, HIGH);
  digitalWrite(data,LOW);
}
inline void sendRed(int kek)
{

  if(kek > 0)
  {
    digitalWrite(data, HIGH);
  }
  else
  {
    digitalWrite(data,LOW);
  }
    digitalWrite(redClk, HIGH);
    digitalWrite(redClk, LOW);
}

void startColours()
{
  for(int x = 0; x < 10; x++)
  {
    sendRed(1);
    sendGreen(1);
  }
}

void gameOver()
{
  Serial.println("Game Over");
  while(true)
    {
      if(Serial.available()>0)
      {
        Serial.println("Reset");
      }
      curTime = millis();
      while(curTime+125 > millis())
      {
        displayStuff();
      }
      if(digitalRead(13))
      {
        digitalWrite(13, LOW);
      }
      else
      {
        digitalWrite(13, HIGH);
      }
    }
}
bool didCrash()
{
  int kek1=0,kek2=0;
  kek1 = snek[0]/8;
  kek2 = snek[0]%8;
  if(snekBool[kek1][kek2])
    return true;
  else
    return false;
}
void setup() {
  pinMode(A0, INPUT);
  randomSeed(analogRead(A0));
  Serial.begin(9600);
  for(short x = 0; x < 8; x++)
  {
    for (short y = 0; y < 8; y++)
    {
      snekBool[x][y] = FA;
    }
  }
  //Serial.println("topkek");
  /*{{FA, FA, FA, FA, FA, FA, FA, FA},
  {FA, FA, FA, FA, FA, FA, FA, FA},
  {FA, FA, FA, FA, FA, FA, FA, FA},
  {FA, FA, FA, FA, FA, FA, FA, FA},
  {FA, FA, FA, FA, FA, FA, FA, FA},
  {FA, FA, FA, FA, FA, FA, FA, FA},
  {FA, FA, FA, FA, FA, FA, FA, FA},
  {FA, FA, FA, FA, FA, FA, FA, FA},};*/


  for(int x = 2; x < 20; x++)
  {
    pinMode(x, OUTPUT);
    digitalWrite(x, LOW);
  }
  digitalWrite(reset, HIGH);
  digitalWrite(plusLclk, HIGH);
  pinMode(leftPin, INPUT_PULLUP);
  pinMode(rightPin, INPUT_PULLUP);
  //delay(1);
  startColours();
  snek[0]= starts[random(0, 15)];    //first snek dot
  smer = random(0,3);
  toLenghten = 3;
  Serial.println("Begin");
  makeCollectible();
}

void loop() {
  //Serial.println(snek[0]);
  digitalWrite(13, HIGH);
  if(snek[0] == collectible)
  {
    makeCollectible();
    snekLength++;
  }

  boolToShort();

  //Serial.println("NEXT");
  leftLatch = false;
  rightLatch = false;
  curTime = millis();
  while(curTime+60 > millis()){displayStuff();}
  while(curTime+oneStep > millis())
  {
    displayStuff();
    shiftOut(data, plusSclk, MSBFIRST, 0);
    digitalWrite(plusLclk, LOW);
    digitalWrite(plusLclk, HIGH);
    goLeft = !digitalRead(leftPin);
    if(goLeft)
      {leftLatch=true;}
    if(rightLatch && leftLatch)   //left is more actual
    {
      rightLatch = false;
    }
    goRight = !digitalRead(rightPin);
    if(goRight)
      {rightLatch=true;}
    if(rightLatch && leftLatch)
    {
      leftLatch = false;     //right is more actual
    }
  }//wait one turn
  digitalWrite(13,LOW);

  for (int x = 62; x > -1; x--)   //snek "moving"
  {
    snek[x+1]=snek[x];
  }


  if(leftLatch)          //course changes
  {
    smer++;
    if(smer == 4)
      {smer = 0;}
  }
  if(rightLatch)
  {
    smer--;
    if(smer == -1)
      {smer = 3;}
  }

  switch (smer)
  {
    case 0: if(snek[0]/8<1 ) {gameOver();}     //up
              else{snek[0] = snek[1] - 8;}
              break;
    case 1: if(snek[0]%8==0) {gameOver();}    //left
              else{snek[0] = snek[1] - 1;}
              break;

    case 2:
              if(snek[0]/8>6) {gameOver();}   //down
              else{snek[0] = snek[1] + 8;}
              break;

    case 3: if(snek[0]%8==7) {gameOver();}    //right
              else{snek[0] = snek[1] +1;}
              break;                              //change smer a detekuj GameOver
  }
  if(didCrash())
  {
    gameOver();
  }

  nowCol = snek[0]%8;
  nowRow = snek[0]/8;
  snekBool[nowRow][nowCol] = true;
  nowCol = snek[snekLength]%8;
  nowRow = snek[snekLength]/8;
  snekBool[nowRow][nowCol] = false;

  turns++;
  if(turns%20==0 & oneStep > 300)//make game faster every 20 turns
  {oneStep = oneStep/1.2;}


}
