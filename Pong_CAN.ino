#include <FlexCAN.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_DC 6
#define OLED_CS 10
#define OLED_RESET 5
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

#if (SSD1306_LCDHEIGHT != 64)
#error("Feil høyde! Vennligst rett i Adafruit_SSD1306.h til 128x64 pixler!");
#endif

void checkMaster();
void myPaddleMove();
void ballMove();
void txBall();
void txMaster();
void txMyPaddle();
void score(int team);
void drawGame();

enum GameRegister{starting, master, slave};
GameRegister gameReg;

CAN_message_t txMsg, rxMsg;

const int joy = 19;
const int joyUp = 22;
const int joyDown = 23;

bool joyClick = true;
bool joyUpClick = true;
bool joyDownClick = true;

int8_t myPaddlePosY = 20;
int8_t theirPaddlePosY = 20;
int8_t ballPosX = 100;
int8_t ballPosY = 30;
int8_t myScore = 0;
int8_t theirScore = 0;

float ballExactX = 110;
float ballExactY = 30;
float ballVelX = -0.5;
float ballVelY = 0.2;

unsigned long progTime;

void setup() 
{
  Can0.begin(500000);
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  display.println("  MAS234 - Gruppe 3");
  display.println(" ");
  display.println(" ");
  display.setTextSize(2);
  display.println(" ~ PONG ~");
  display.display();
  
  gameReg = starting;
  pinMode(joy, INPUT_PULLUP);
  pinMode(joyUp, INPUT_PULLUP);
  pinMode(joyDown, INPUT_PULLUP);
}

void loop() 
{
  progTime = millis();
  switch(gameReg) 
  {
    case starting : checkMaster();
      break;
      
    case master : ;
      myPaddleMove();
      while (Can0.available())
      {
        Can0.read(rxMsg);
        if ((rxMsg.id < 0x32) && (rxMsg.id > 0x13))
        {
          theirPaddlePosY = rxMsg.buf[0];
        }
      }
      ballMove();
      txBall();
      txMyPaddle();
      drawGame();
      while (millis() < (progTime + 10)) {
        delay(1);
      }
      break;
      
    case slave : ;
      myPaddleMove();
      txMyPaddle();
      while (Can0.available())
      {
        Can0.read(rxMsg);
        if ((rxMsg.id < 0x32) && (rxMsg.id > 0x13))
        {
          theirPaddlePosY = rxMsg.buf[0];
        }
        else if (rxMsg.id > 50)
        {
          ballPosX = (127 - rxMsg.buf[0]); // Ballens posisjon er invertert for å vise korrekt på display. 
          ballPosY = rxMsg.buf[1];
        }
      }
      drawGame();
      if ((ballPosX > 122) && ((ballPosY < myPaddlePosY) || (ballPosY > (myPaddlePosY + 20)))) // Gruppe 4 har disse randbetingelsene.
      {
        score(0); // Du scoret!
      }
      else if ((ballPosX < 5) && ((ballPosY < theirPaddlePosY) || (ballPosY > (theirPaddlePosY + 20)))) // Gruppe 4 har disse randbetingelsene.
      {
        score(1); // De andre scoret...
      }
      while (millis() < (progTime + 10)) {
        delay(1);
      }
      break;
  }
}

// Sjekk hvem som er master og slave etter oppstart før spillet setter i gang. 
void checkMaster()
{
  joyClick = digitalRead(joy);
  if (joyClick == false) 
  {
    joyClick = true;
    Serial.println("Du er master. Spillet starter!");
    gameReg = master;
    txMaster(); // Gruppe 4 har brukt id = 0 som master melding, txMaster() sender denne. 
    txBall();
    drawGame();
    delay(300);
  }
  else if (Can0.available()){
    Can0.read(rxMsg);
    if ((rxMsg.id == 0x0) || rxMsg.id > 33) // Gruppe 4 har brukt id = 0 som master melding.
    {
      Serial.println("Du er slave. Spillter starter!");
      ballPosX = rxMsg.buf[0];
      ballPosY = rxMsg.buf[1];
      gameReg = slave;
      drawGame();
    }
  }
}

// Registrer og oppdaterer posisjon av din paddle.
void myPaddleMove()
{
  joyUpClick = digitalRead(joyUp);
  joyDownClick = digitalRead(joyDown);
  if (joyUpClick == false) 
  {
    joyUpClick = true;
    if (myPaddlePosY == 1)
    {
      myPaddlePosY = 1;
    }
    else 
    {
      myPaddlePosY--;
    }
  } 
  else if (joyDownClick == false)
  {
    joyDownClick = true;
    if (myPaddlePosY == (63 - 20)) 
    {
      myPaddlePosY = (63 - 20);
    }
    else 
    {
      myPaddlePosY++;
    }
  }
}

void ballMove()
{
  ballExactX += ballVelX;
  ballExactY += ballVelY;
  ballPosX = static_cast<int8_t>(round(ballExactX)); // Posisjon i heltall for displayet. 
  ballPosY = static_cast<int8_t>(round(ballExactY)); 
  if (ballPosX > 120 && ballPosY >= myPaddlePosY && ballPosY <= (myPaddlePosY + 20))
  {
    ballVelX = -ballVelX; // Ballen treffer din paddle. 
  }
  else if (ballPosX < 7 && ballPosY >= theirPaddlePosY && ballPosY <= (theirPaddlePosY + 20))
  {
    ballVelX = -ballVelX; // Ballen treffer deres paddle.
  }
  if (ballPosX > 126)  
  {
    score(0); // Du scoret!  
  }
  else if (ballPosX < 1) 
  {
    score(1); // De andre scoret...
  }
  if (ballPosY < 3 || ballPosY > 60)
  {
    ballVelY = -ballVelY; // Ballen treffer øvre/nedre vegg.
  }
}

// Send CAN melding med ballposisjon.
void txBall() 
{
  txMsg.id = 0x35; // 53 i desimaltall (50 + group no. 3)
  txMsg.len = 2;
  txMsg.buf[0] = ballPosX; 
  txMsg.buf[1] = ballPosY; 
  Can0.write(txMsg);
}

// Send CAN melding med master signal (lagt til for kompatibilitet med gruppe 4. 
void txMaster() 
{
  txMsg.id = 0x0; 
  txMsg.len = 1;
  txMsg.buf[0] = 1;  
  Can0.write(txMsg);
}

// Send CAN medling med posisjon av din paddle. 
void txMyPaddle() 
{
  txMsg.id = 0x17; // 23 i desimaltall (20 + gruppenummer 3)
  txMsg.len = 1;
  txMsg.buf[0] = myPaddlePosY;
  Can0.write(txMsg);
}

// Tell mål og vis på displayet.
void score(int team) 
{
  if (team == 0) 
  {
    myScore++;
    ballExactX = 110;
    ballExactY = 30;
    ballVelX = -0.5;
    ballVelY = 0.2;
    myPaddlePosY = 20;
  }
  else if (team == 1) 
  {
    theirScore++;
    ballExactX = 20;
    ballExactY = 30;
    ballVelX = 0.5;
    ballVelY = -0.2;
    myPaddlePosY = 20;
  }
  display.println(" ");
  display.setTextSize(2);
  display.print("  Score ");
  display.setTextSize(1);
  display.println(" ");
  display.println(" ");
  display.println(" ");
  display.print("   Dine poeng: ");
  display.println(myScore);
  display.print("   Deres poeng: ");
  display.println(theirScore);
  display.display();
  if (gameReg == master) 
  {
    delay(2000);
  }
  else if (gameReg == slave) 
  {
    delay(2050);
  }
}

// Tegn spillbrettet på displayet.
void drawGame() 
{
  display.clearDisplay();  
  display.drawRect(0, 0, 128, 64, WHITE);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.fillCircle(ballPosX, ballPosY, 2, WHITE);
  display.fillRect(123, myPaddlePosY, 2, 20, WHITE);
  display.fillRect(3, theirPaddlePosY, 2, 20, WHITE);
  display.display();
}
