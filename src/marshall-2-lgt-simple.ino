//ver 2: MiniEVB variant, no EEPROM feature

#include "SSD1306AsciiAvrI2c.h"
#include "dosfont.h"
#define I2C_ADDRESS 0x3C
SSD1306AsciiAvrI2c oled;

/* AltSoftSerial
  Nano:
  Receive: 8 (D8) ICP1
  Transmit: 9 (D9) OC1A
*/
/* MAX485 connection:
   TX: DI >D9
   RX: RO >D8
   DE+RE: >D7, usually HIGH
   A: + (straight)
   B: - (inverse)
*/
#include <AltSoftSerial.h>
AltSoftSerial mySerial;
#define TXEN 7  //D7 pin

//let's initialize the VISCA commands
const byte arMenu[4] = {0x01, 0x06, 0x06, 0x02};
const byte arUp[4]   = {0x01, 0x06, 0x06, 0x11};
const byte arDown[4] = {0x01, 0x06, 0x06, 0x12};
const byte arLeft[4] = {0x01, 0x06, 0x06, 0x14};
const byte arRight[4] = {0x01, 0x06, 0x06, 0x18};

// Encoder
#define ENC_S1 4 //D4
#define ENC_S2 5 //D5
#define ENC_SW 6 //D6
#include "encMinim.h"
// пин clk, пин dt, пин sw, направление (0/1)
encMinim enc(ENC_S1, ENC_S2, ENC_SW, 1);

#define DEF_CAM 1; //default cam
byte camnum = DEF_CAM;            //camera number. valid values 1 to 7
byte prevnum = 0;

#define TOT_BAUDS 6
#define DEF_BAUDRATE 0  //default baudrate
const long baudrates[TOT_BAUDS] = {9600, 14400, 19200, 38400, 57600, 115200};
int8_t baudrate = DEF_BAUDRATE; //9600 in array above
int8_t prevbaud = -1; //sometimes it drops to -1 so we need the signed type

#define LED_BUILTIN 13

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TXEN, OUTPUT);
  digitalWrite(TXEN, HIGH); //enable TX
  
  

  //display setup
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
digitalWrite(LED_BUILTIN, HIGH);
  oled.setFont(dosfont);
  //initial logo
  oled.clear();
  oled.println(F("M A R S H A L L\ntiny RCP\nby Kirill Ageyev\n(c) 2021"));
  delay(1000);

  //camera selection
  oled.clear(); oled.set1X();
  oled.println(F("Camera:"));
  do {
    enc.tick();

    if (enc.isLeft()) camnum--;
    if (enc.isRight()) camnum++;
    if (camnum < 1) camnum = 7;
    if (camnum > 7) camnum = 1;

    //camnum changed, displaying routine
    if (camnum != prevnum) {
      oled.setCursor(0, 2);  oled.set2X();
      oled.print(camnum);
    }

    //remembering the camera number to check whether it's changed in next iteration
    prevnum = camnum;
  } while (enc.isClick() == false);

  //baud rate selection
  oled.clear();  oled.set1X();
  oled.println(F("Baud rate:"));
  do {
    enc.tick();

    if (enc.isLeft()) baudrate--;
    if (enc.isRight()) baudrate++;
    if (baudrate < 0) baudrate = TOT_BAUDS - 1;
    if (baudrate > TOT_BAUDS - 1) baudrate = 0;

    //baudrate changed, displaying routine
    if (baudrate != prevbaud) {
      oled.setCursor(0, 2);  oled.set2X();
      oled.print(baudrates[baudrate]);
      oled.print(F("   ")); //extra spaces for overwriting the shorter values on screen
    }

    //remembering the baudrate to check whether it's changed in next iteration
    prevbaud = baudrate;
  } while (enc.isClick() == false);

  //okay, we've got the baud rate and camera#, now it's time to display some inctructions and proceed to loop cycle

  /* 0x18: arrow up
     0x19: arrow down
     0x1A: arrow right
     0x1B: arrow left */
     
  //max string length 16, lines 4
  oled.clear();  oled.set1X();
  oled.print(F("c#:")); oled.print(camnum);
  oled.print(F(" baud:")); oled.println(baudrates[baudrate]);

  oled.println(F("Turn to UP\x12\DOWN\nHold+turn to L\x1DR\nClick to select"));

  //serial setup
//  Serial.begin(baudrates[baudrate]);
  mySerial.begin(baudrates[baudrate]);
} //of setup()

void loop() {
  enc.tick();
  if (enc.isLeft()) cmd(camnum, arUp, 4);
  if (enc.isRight()) cmd(camnum, arDown, 4);
  if (enc.isLeftH()) cmd(camnum, arLeft, 4);
  if (enc.isRightH()) cmd(camnum, arRight, 4);
  if (enc.isClick()) cmd(camnum, arMenu, 4);
} //of loop()

//#define DEBUG
//executing the VISCA command
void cmd(byte cam, byte str[], byte len) {
#ifdef DEBUG
  Serial.write(0x80 + cam);
  Serial.write(str, len);
  Serial.write(0xFF);
#else
  mySerial.write(0x80 + cam);
  mySerial.write(str, len);
  mySerial.write(0xFF);
#endif

  //toggle built-in led
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 1);
}
