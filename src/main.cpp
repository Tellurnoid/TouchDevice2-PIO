
//Alt + Ctrl + U   UP

#include <Arduino.h>
#define FS_NO_GLOBALS
#include <FS.h>
#include "SPIFFS.h" // ESP32 only
#include "BluetoothSerial.h"
#include <TJpg_Decoder.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <TFT_eSPI.h> 
TFT_eSPI tft = TFT_eSPI(); 
#define TFT_GREY 0x7D5A
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// The STMPE610 uses hardware SPI on the shield, and #8

// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 40
#define PENRADIUS 3
/*
#define CS_PIN    0
#define MOSI_PIN 23
#define MISO_PIN 19
#define SCK_PIN  18
#define TIRQ_PIN  2
*/
#define CS_PIN 0
#define TIRQ_PIN 2

XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling
int oldcolor, currentcolor;
bool IsOpenPalette = false;
bool IsTouch = false;

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
void drawHomeMenu(void);
void drawChatMenu(void);
void DrawStartMenu_Dots(void);
void DrawStartMenu_Dots(void);
void drawPaintGrid(void);
void PaintApp(void);
void BTChat(void);
unsigned long BluetoothText(void);

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

int data = 0;
int app = 0;

void drawHomeMenu(void){
  uint16_t w = 0, h = 0;
  tft.fillScreen(ILI9341_WHITE);////////////////////ホーム画面絵画
  for(int i=0; i<3; i++){
    tft.drawCircle(random(240), random(320), random(10,100),random(0xFFFF));
  }
  tft.fillRect(0,0,240,BOXSIZE,TFT_LIGHTGREY);
  TJpgDec.getFsJpgSize(&w, &h, "Ico_Paint.jpeg"); 
  TJpgDec.drawFsJpg(0, BOXSIZE, "/Ico_Paint.jpeg");
  TJpgDec.drawFsJpg(60, BOXSIZE, "/Ico_BTChat.jpeg");
}
void drawChatMenu(void){
  uint16_t w = 0, h = 0;
       tft.setTextColor(TFT_WHITE,TFT_BLACK);
     tft.setTextSize(1);
        TJpgDec.getFsJpgSize(&w, &h, "AloneWithSky.jpeg"); 
      TJpgDec.drawFsJpg(0, 0, "/AloneWithSky.jpeg");
    tft.fillRect(0, 320-BOXSIZE, 120, BOXSIZE, ILI9341_GREEN);
     tft.setTextColor(TFT_WHITE,TFT_GREEN);
  
     tft.drawCentreString("YES", 60, 300,2);

     tft.fillRect(120, 320-BOXSIZE, 120, BOXSIZE, ILI9341_RED);
     tft.setTextColor(TFT_WHITE,TFT_RED);
     tft.drawCentreString("NO", 180, 300,2);
     tft.setTextColor(TFT_BLACK,0x7d5a);
     tft.setTextSize(2);
}
void DrawStartMenu_Dots(void){
  tft.fillScreen(ILI9341_BLACK);
  for(int i=0; i<200; i++){
    tft.drawPixel(random(320),random(320),TFT_WHITE);
  }
  for(int i=0; i<3; i++){
    tft.drawCircle(random(240), random(320), random(10,100),random(0xFFFF));
    //delay(20);
  }
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.setTextSize(2);
  tft.drawCentreString("Tellurnoid", 160, 93,2);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.setTextSize(3);
}
void drawPaintGrid(void){
    //tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_RED);
  tft.fillRect(BOXSIZE, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_YELLOW);
  tft.fillRect(BOXSIZE*2, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_GREEN);
  tft.fillRect(BOXSIZE*3, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_CYAN);
  tft.fillRect(BOXSIZE*4, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_BLUE);
  tft.fillRect(BOXSIZE*5, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_MAGENTA);
  //tft.drawRect(0, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_WHITE);
  currentcolor = ILI9341_RED;
  tft.drawCentreString("Cloud2.jpeg", 110, 80,2);
  tft.drawCentreString("FoldBlue.jpeg", 110, 120,2);
  tft.drawCentreString("FoldHighway1.jpeg", 110, 160,2);
  tft.drawCentreString("Park1.jpeg", 110, 200,2);
}
void PaintApp(void){
  TS_Point p = ts.getPoint();
  // Scale from ~0->4000 to tft.width using the calibration #'s
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_GREEN,TFT_BLACK);
  tft.setTextSize(1);
  if(IsOpenPalette == false){
     if (((p.y-PENRADIUS) > BOXSIZE) && ((p.y+PENRADIUS) < tft.height())) {
     //if (!((p.y-PENRADIUS) > BOXSIZE) && ((p.y+PENRADIUS) < BOXSIZE)) {
       tft.fillCircle(p.x, p.y, PENRADIUS, currentcolor);
      }
     if(p.x < BOXSIZE && p.y < BOXSIZE){
       IsOpenPalette = true;
       drawPaintGrid();
      }
  }

  if(IsOpenPalette == true){
      if(p.x>BOXSIZE && p.y<BOXSIZE){
        IsOpenPalette = false;
        //tft.fillRect(0,320-BOXSIZE,BOXSIZE*6 ,BOXSIZE,TFT_BLACK);
        tft.fillScreen(ILI9341_BLACK);
       // tft.fillRect(0,0,BOXSIZE,BOXSIZE,TFT_WHITE);
       }
      if (p.y > 320-BOXSIZE*2) {
        oldcolor = currentcolor;
       if (p.x < BOXSIZE) { 
         currentcolor = ILI9341_RED; 
         tft.drawRect(0, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_WHITE);
       } else if (p.x < BOXSIZE*2) {
         currentcolor = ILI9341_YELLOW;
         tft.drawRect(BOXSIZE, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_WHITE);
       } else if (p.x < BOXSIZE*3) {
         currentcolor = ILI9341_GREEN;
         tft.drawRect(BOXSIZE*2, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_WHITE);
       } else if (p.x < BOXSIZE*4) {
         currentcolor = ILI9341_CYAN;
         tft.drawRect(BOXSIZE*3, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_WHITE);
       } else if (p.x < BOXSIZE*5) {
         currentcolor = ILI9341_BLUE;
         tft.drawRect(BOXSIZE*4, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_WHITE);
       } else if (p.x < BOXSIZE*6) {
         currentcolor = ILI9341_MAGENTA;
         tft.drawRect(BOXSIZE*5, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_WHITE);
     }

     if (oldcolor != currentcolor) {
        if (oldcolor == ILI9341_RED) 
          tft.fillRect(0, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_RED);
        if (oldcolor == ILI9341_YELLOW) 
          tft.fillRect(BOXSIZE, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_YELLOW);
        if (oldcolor == ILI9341_GREEN) 
          tft.fillRect(BOXSIZE*2, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_GREEN);
        if (oldcolor == ILI9341_CYAN) 
          tft.fillRect(BOXSIZE*3, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_CYAN);
        if (oldcolor == ILI9341_BLUE) 
          tft.fillRect(BOXSIZE*4, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_BLUE);
        if (oldcolor == ILI9341_MAGENTA) 
          tft.fillRect(BOXSIZE*5, 320-BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_MAGENTA);
     }
  }
  if (p.y < 320-BOXSIZE*2 && p.y > BOXSIZE) {
         if(p.y<BOXSIZE*2)
          TJpgDec.drawFsJpg(0, 0, "/Cloud2.jpeg");
    else if(p.y<BOXSIZE*3)
          TJpgDec.drawFsJpg(0, 0, "/FoldBlue.jpeg");
    else if(p.y<BOXSIZE*4)
          TJpgDec.drawFsJpg(0, 0, "/FoldHighway1.jpeg");
    else if(p.y<BOXSIZE*5)
          TJpgDec.drawFsJpg(0, 0, "/Park1.jpeg");
  }

 }
}
void BTChat(void){
  TS_Point p = ts.getPoint();
  // Scale from ~0->4000 to tft.width using the calibration #'s
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
        if (SerialBT.available()) {
       data = SerialBT.read();
       if(data == '0'){tft.print("0");}
       else
       {
         tft.setTextColor(TFT_BLACK,TFT_GREY);
  //tft.print(data);
       if(data == '1'){tft.print("1");}
       if(data == '2'){tft.print("2");}
       if(data == '3'){tft.print("3");}
       if(data == '4'){tft.print("4");}
       if(data == '5'){tft.print("5");}
       if(data == '6'){tft.print("6");}
       if(data == '7'){tft.print("7");}
       if(data == '8'){tft.print("8");}
       if(data == '9'){tft.print("9");}

       if (data == 'a') {tft.print("a");}      if (data == 'b') {tft.print("b");}      if (data == 'c') {tft.print("c");}
       if (data == 'd') {tft.print("d");}      if (data == 'e') {tft.print("e");}      if (data == 'f') {tft.print("f");}
       if (data == 'g') {tft.print("g");}      if (data == 'h') {tft.print("h");}      if (data == 'i') {tft.print("i");}
       if (data == 'j') {tft.print("j");}      if (data == 'k') {tft.print("k");}      if (data == 'l') {tft.print("l");}
       if (data == 'm') {tft.print("m");}      if (data == 'n') {tft.print("n");}      if (data == 'o') {tft.print("o");}
       if (data == 'p') {tft.print("p");}      if (data == 'q') {tft.print("q");}      if (data == 'r') {tft.print("r");}
       if (data == 's') {tft.print("s");}      if (data == 't') {tft.print("t");}      if (data == 'u') {tft.print("u");}
       if (data == 'v') {tft.print("v");}      if (data == 'w') {tft.print("w");}      if (data == 'x') {tft.print("x");}
       if (data == 'y') {tft.print("y");}      if (data == 'z') {tft.print("z");}
       
       if(data == 'A'){tft.print("A");}        if(data == 'B'){tft.print("B");}        if(data == 'C'){tft.print("C");}
       if(data == 'D'){tft.print("D");}        if(data == 'E'){tft.print("E");}        if(data == 'F'){tft.print("F");}
       if(data == 'G'){tft.print("G");}        if(data == 'H'){tft.print("H");}        if(data == 'I'){tft.print("I");}
       if(data == 'J'){tft.print("J");}        if(data == 'K'){tft.print("K");}        if(data == 'L'){tft.print("L");}
       if(data == 'M'){tft.print("M");}        if(data == 'N'){tft.print("N");}        if(data == 'O'){tft.print("O");}
       if(data == 'P'){tft.print("P");}        if(data == 'Q'){tft.print("Q");}        if(data == 'R'){tft.print("R");}
       if(data == 'S'){tft.print("S");}        if(data == 'T'){tft.print("T");}        if(data == 'U'){tft.print("U");}
       if(data == 'V'){tft.print("V");}        if(data == 'W'){tft.print("W");}        if(data == 'X'){tft.print("X");}
       if(data == 'Y'){tft.print("Y");}        if(data == 'Z'){tft.print("Z");}
       
       if(data == '!'){tft.print("!");}        if(data == '?') {tft.print("?");}       if(data == '+'){tft.print("+");}
       if(data == '@'){tft.print("@");}        if(data == '*'){tft.print("*");}        if(data == '{'){tft.print("{");}
       if(data == '}'){tft.print("}");}        if(data == '='){tft.print("=");}        if(data == '-'){tft.print("-");}
       if(data == '~'){tft.print("~");}        if(data == '|'){tft.print("|");}        if(data == '&'){tft.print("&");}
       if(data == '%'){tft.print("%");}        if(data == ';'){tft.print(";");}        if(data == '/'){tft.print("/");}
       if(data == '<'){tft.print("<");}        if(data == '>'){tft.print(">");}        if(data == ','){tft.print(",");}  
       if(data == ' '){tft.print(" ");}        if(data == '['){tft.print("[");}        if(data == ']'){tft.print("]");}
       if(data == '#'){tft.print("#");}        if(data == '$'){tft.print("$");}        
       
       if(data == ' '){tft.print(" ");delay(1);}
       if(data == '.'){tft.print(".");delay(1);}
       if(data == ':'){tft.println("");delay(1);}
       if(data == ';'){
        // tft.fillRect(0, 0, 240, 320-BOXSIZE, ILI9341_BLACK);
        drawChatMenu();
        tft.setCursor(0,0);
         }
       }
    }
    if(p.y>0 && p.y<BOXSIZE*2 && p.z>100){
      //tft.fillRect(0, 0, 240, 320-BOXSIZE, ILI9341_BLACK);
     drawChatMenu();
     tft.setCursor(0,0);
      }
    if(p.y>320-BOXSIZE && p.y<320 && p.z>100){
    if(p.x>116 && p.x<124){}
    else if(p.x<115){SerialBT.println("YES");tft.setTextColor(TFT_DARKGREEN,TFT_GREY);tft.println("");tft.println("YES");delay(500);}
    else if(p.x>125){SerialBT.println("NO");tft.setTextColor(TFT_RED,TFT_GREY);tft.println("");tft.print("NO");delay(500);}
   }

}
unsigned long BluetoothText(void) {
  unsigned long start = micros();
  tft.print(SerialBT.read()-48);
  return micros() - start;
}

void setup(){
  Serial.begin(115200);
  SerialBT.begin("ESP32"); //Bluetooth device name
  Serial.println(F("Touch Paint!"));
  tft.begin();
  ts.setRotation(2);
  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  Serial.println("Touchscreen started");
  
  tft.fillScreen(ILI9341_BLACK);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  randomSeed(millis());
  DrawStartMenu_Dots();
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  tft.setSwapBytes(true); // We need to swap the colour bytes (endianess)
  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(tft_output);
  //delay(1500);
  // See if there's any  touch data for us
  // if (ts.bufferEmpty()) {return;}

   TS_Point p = ts.getPoint();
   p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
   p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
   if(p.x < 120 && p.y < BOXSIZE){
     app=2;
     tft.fillRect(0, 0, 120, BOXSIZE, ILI9341_BLUE);
     delay(600); 

     tft.fillScreen(ILI9341_BLACK);
     drawChatMenu();
     }else if(p.x > 120 && p.y < BOXSIZE){
       app=1;
       tft.fillRect(120, 0, 120, BOXSIZE, ILI9341_GREEN);
       delay(600);
       tft.fillScreen(ILI9341_BLACK);
          }//1:ペイント 2:Chat
      else{
        app=0;
        drawHomeMenu();
      }
}


void loop() {
  uint16_t w = 0, h = 0;
  for(;;) 
  {
  // See if there's any  touch data for us
   if (ts.bufferEmpty()) {
     return;
     }
   TS_Point p = ts.getPoint();
   p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
   p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
  
  if(app==1){PaintApp();}
  else if(app==2){BTChat();}
  
  else if(app==0){
  if((p.x>0 && p.x<60) && (p.y>BOXSIZE && p.y<BOXSIZE*2+60) && p.z>100){app=1;drawPaintGrid();}
  else if((p.x>BOXSIZE*4 && p.x<BOXSIZE*4+60) && (p.y>BOXSIZE*4 && p.y<BOXSIZE*4+60) && p.z>100){app=2;drawChatMenu();}
  tft.setTextColor(TFT_BLUE,TFT_WHITE);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(1);
  tft.drawNumber(p.z, 150, 200,2);
  delay(50);
  tft.fillRect(130, 190, 40, 20, TFT_WHITE);
  }
 }
}