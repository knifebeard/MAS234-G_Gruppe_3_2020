#include <FlexCAN.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_DC 6
#define OLED_CS 10
#define OLED_RESET 5
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

static CAN_message_t txMsg, rxMsg;
String inString; 
int rxCount(0);

String inMsgString(CAN_message_t inMsg);
void displayFunc(int rx, int id); 

void setup() {
  Can0.begin(500000);
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();

  delay(500);

  display.setTextSize(0);
  display.setTextColor(WHITE);
  display.setCursor(0,5);
  display.println("Starter CAN-program");
  display.println(" ");
  display.println("Sender 'hei' i hex.");
  display.println("Viser ant mottatte"); 
  display.println("meldinger og ID for "); 
  display.println("den siste.");
  display.display();

  delay(5000);
}

void loop() {
Serial.println("Sending txMsg. ");
  txMsg.id = 0x1;
  txMsg.len = 3;
  txMsg.buf[0] = 0x68;
  txMsg.buf[1] = 0x65;
  txMsg.buf[2] = 0x69;
  Can0.write(txMsg);
  delay(100);

  while(Can0.available() > 0) 
  {
    Serial.print("Receiving rxMsg: ");
    Can0.read(rxMsg);
    inString = inMsgString(rxMsg);
    Serial.print(inString);
    Serial.print("\n\n");
    rxCount++;
    displayFunc(rxCount, rxMsg.id);
    delay(200);
  }
}

String inMsgString (CAN_message_t inMsg) {
  char charValue[8] = {' ',' ',' ',' ',' ',' ',' ',' '};
  const int lengthMsg = static_cast<int>(inMsg.len); 
  String outMsg; 
  
  for(int ii = 0; ii < lengthMsg; ii++)
  {
    charValue[ii] = static_cast<char>(inMsg.buf[ii]);
    outMsg += static_cast<String>(charValue[ii]);
  }
  return(outMsg);
}

void displayFunc (int rx, uint32_t id) {
  display.clearDisplay();
  display.drawRoundRect(0, 0, 128, 64, 12, WHITE);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  display.println("  MAS234 - Gruppe 3");
  display.println(" ");
  display.println(" CAN-statistikk");
  display.println(" ------------------");
  display.print(" Antall mottatt: ");
  display.println(rx);
  display.print(" Mottok sist ID: ");
  display.println(id);
  display.display();
}
