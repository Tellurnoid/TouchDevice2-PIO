
#include <Arduino.h>
#define FS_NO_GLOBALS
#include <FS.h>
  #include "SPIFFS.h" // ESP32 only
#include <WiFi.h>
#include <time.h>
#define JST     3600* 9
#define USE_DMA

int oldTime;
// Include SD


#ifdef USE_DMA
  uint16_t  dmaBuffer1[16*16]; // Toggle buffer for 16*16 MCU block, 512bytes
  uint16_t  dmaBuffer2[16*16]; // Toggle buffer for 16*16 MCU block, 512bytes
  uint16_t* dmaBufferPtr = dmaBuffer1;
  bool dmaBufferSel = 0;
#endif


#include "SPI.h"
#include "TFT_eSPI.h"
//#include <Wire.h>
#include "Free_Fonts.h"
#include <XPT2046_Touchscreen.h>
#include <IRremote.hpp>
#include "PinDefinitionsAndMore.h"
#include <TJpg_Decoder.h>
int sensorPin = A0;     //アナログ0番ピンを指定
int sensorValue = 0;

TFT_eSPI tft = TFT_eSPI();
unsigned long drawTime = 0;

#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000
#define BOXSIZE 40
#define PENRADIUS 3
#define CS_PIN 0
#define TIRQ_PIN 2
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);
int oldcolor, currentcolor;
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);

#include <SD.h>
SPIClass spiSD(HSPI);
#define SD_SCK  14
#define SD_MISO 33
#define SD_MOSI 13
#define SD_SS   15
#define BTSIZE 80
#define halfWidth 120
#define halfHeight 160
#define one_thirdBOXSIZE 106
  const char* ssid = "your ssidA";
  const char* password = "your passwardA";

uint16_t ShTColor = TFT_WHITE;
uint16_t ShDColor = TFT_WHITE;
  int sleepTimer=0;
  int app=2;
  int AlarmHour=6;
  int AlarmMin =0;
  bool IsAlarmON = true;
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;
#ifdef USE_DMA
  if (dmaBufferSel) dmaBufferPtr = dmaBuffer2;
  else dmaBufferPtr = dmaBuffer1;
  dmaBufferSel = !dmaBufferSel; 
  tft.pushImageDMA(x, y, w, h, bitmap, dmaBufferPtr);
#else
  tft.pushImage(x, y, w, h, bitmap); 
#endif
  return 1;
}
void DrawStartMenu_Dots(void){
  tft.fillScreen(ILI9341_BLACK);
  for(int i=0; i<200; i++){
    tft.drawPixel(random(TFT_HEIGHT),random(TFT_HEIGHT),TFT_WHITE);
  }
  for(int i=0; i<3; i++){
    tft.drawCircle(random(TFT_WIDTH), random(TFT_HEIGHT), random(10,100),random(0xFFFF));
    //delay(20);
  }
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  //tft.setTextSize(2);
  tft.setFreeFont(FF20);
  tft.setCursor(30,83);
  tft.println("Tellurnoid");
}
void draw_RemoteLightGrid(void){
  #define BTSIZE 80
  tft.fillRect(0,40,240,320-80,TFT_BLACK);
  tft.fillRoundRect(0,40,TFT_WIDTH,66,10,TFT_GRAY);
  tft.fillRoundRect(0,one_thirdBOXSIZE,halfWidth,one_thirdBOXSIZE,10,TFT_GRAY);
  tft.fillRoundRect(halfWidth,one_thirdBOXSIZE,halfWidth,one_thirdBOXSIZE,10,TFT_GRAY);
  tft.fillRoundRect(0,212,TFT_WIDTH,66,10,TFT_GRAY);
  tft.fillCircle(220,15,10,TFT_GREENYELLOW);
  tft.drawRect(0,40,TFT_WIDTH,66,TFT_BLACK);
  tft.drawRect(0,one_thirdBOXSIZE,halfWidth,one_thirdBOXSIZE,TFT_BLACK);
  tft.drawRect(halfWidth,one_thirdBOXSIZE,halfWidth,one_thirdBOXSIZE,TFT_BLACK);
  tft.drawRect(0,212,TFT_WIDTH,66,TFT_BLACK);
  tft.setTextColor(TFT_BLUE);
  tft.setFreeFont(FF21);
  tft.drawCentreString("ON/OFF",halfWidth,73,2);
  tft.drawCentreString("Night",halfWidth,245,2);
  tft.drawCentreString("Brighter",180,159,2);
  tft.drawCentreString("Darker",60,159,2);
        tft.drawRoundRect(0,285,240,35,2,TFT_DARKCYAN);
        tft.setTextColor(TFT_WHITE,TFT_BLACK);
        tft.drawCentreString("Sleep",115,297,2);
        tft.setTextColor(TFT_BLUE);
  
}
void draw_AlarmClock(void){
  tft.fillRect(0,40,240,320-40,TFT_BLACK);
  tft.setFreeFont(FF21);
  tft.setTextColor(TFT_BLUE,TFT_BLACK);
  tft.drawCentreString("[Alarm Clock]",120,45,1);
  tft.setFreeFont(FF20);
  tft.fillCircle(220,15,10,TFT_RED);//閉じる
  tft.drawRoundRect(150,245,85,30,10,TFT_GREEN);
  if(IsAlarmON==true){tft.fillRoundRect(150,245,40,30,10,TFT_GREEN);}
  if(IsAlarmON==false){tft.fillRoundRect(195,245,40,30,10,TFT_GREEN);}
   tft.setTextColor(TFT_BLUE,TFT_BLACK);
       tft.drawCentreString((String)AlarmHour,60,120,1);
       tft.drawCentreString(":",halfWidth,120,1);
       tft.drawCentreString((String)AlarmMin,180,120,1);
        tft.drawRoundRect(0,285,240,35,2,TFT_DARKCYAN);
        tft.setTextColor(TFT_WHITE,TFT_BLACK);
        tft.drawCentreString("Sleep",115,297,2);
        tft.setTextColor(TFT_BLUE);

}
void app_AlarmClock(void){
  tft.setFreeFont(FF20);
  uint16_t w = 0, h = 0;
  time_t t;
  struct tm *tm;
 // tft.setCursor(50, 100);
  static const char *wd[7] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
  t = time(NULL);
  tm = localtime(&t);
   TS_Point p = ts.getPoint();
   p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
   p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
  if(p.z>200){
    digitalWrite(TFT_BL, HIGH);
    tft.setTextColor(TFT_BLUE,TFT_BLACK);
    if(p.y<48 && p.x>210){app=2;draw_RemoteLightGrid();delay(500);}//閉じる
    else if(p.y<120 && p.y>40){//up
      if(p.x<halfWidth){//時間up
        AlarmHour=AlarmHour+1;
        if(AlarmHour>23){AlarmHour=0;}
        tft.fillRect(20,115,90,60,TFT_BLACK);
        tft.drawCentreString((String)AlarmHour,60,120,1);
         delay(100);
        }
      if(p.x>halfWidth){//分up
        AlarmMin=AlarmMin+1;
        if(AlarmMin>60){AlarmMin=0;}
        tft.fillRect(140,115,90,60,TFT_BLACK);
        tft.drawCentreString((String)AlarmMin,180,120,1);
         delay(60);
        }

    }
    else if(p.y<240){//down
      if(p.x<halfWidth){//時間down
        AlarmHour=AlarmHour-1;
        if(AlarmHour<0){AlarmHour=23;}
        tft.fillRect(20,115,90,60,TFT_BLACK);
        tft.drawCentreString((String)AlarmHour,60,120,1);
         delay(100);
        }
      if(p.x>halfWidth){//分down
        AlarmMin=AlarmMin-1;
        if(AlarmMin<0){AlarmMin=60;}
        tft.fillRect(140,115,90,60,TFT_BLACK);
        tft.drawCentreString((String)AlarmMin,180,120,1);
         delay(60);
        }
    }
    else if((p.y>240 && p.y<280) && (p.x>150 &&p.x<195)){//on/off
        IsAlarmON=false;
        tft.fillRoundRect(150,245,85,30,10,TFT_BLACK);
        tft.drawRoundRect(150,245,85,30,10,TFT_GREEN);
        tft.drawRoundRect(195,245,40,30,10,TFT_GREEN);
        tft.setTextColor(TFT_BLUE);
        tft.drawCentreString("off",170,245,2);
      }
      else if((p.y>240 && p.y<280) && (p.x>196 &&p.x<240)){
        IsAlarmON=true;
        tft.fillRoundRect(150,245,85,30,10,TFT_BLACK);
        tft.drawRoundRect(150,245,85,30,10,TFT_GREEN);
        tft.fillRoundRect(150,245,40,30,10,TFT_GREEN);
        tft.setTextColor(TFT_WHITE);
        tft.drawCentreString("on",215,245,2);
        tft.setTextColor(TFT_BLUE);
      }
      if(p.y>285){
        digitalWrite(TFT_BL, LOW);
        sleepTimer=50;
      }
      
    //while(p.z>200){}
    //Serial.println((String)"byte a=" + a + " , float f=" + f + " ,str[]=" + str);
    
  }
  if (tm->tm_min != oldTime){
  tft.startWrite();
  tft.fillRect(0,0,240,40,TFT_BLACK);
  delay(10);
  tft.setCursor(0, 0);
  tft.setTextColor(ShDColor);
  tft.setFreeFont(FF21);
  tft.startWrite();
  tft.println();
  tft.print("    ");
  tft.print(tm->tm_hour);
  tft.print(":");
  if(tm->tm_min<10){tft.print("0");}
  tft.print(tm->tm_min);
  tft.print("    ");
  tft.print(tm->tm_year+1900);
  tft.print(" "); 
  tft.print(tm->tm_mon+1);
  tft.print("/");
  tft.print(tm->tm_mday);
  tft.print("[");
  tft.print(wd[tm->tm_wday]);
  tft.print("]");
  //tft.setFreeFont(FF20);
  tft.endWrite();
     delay(200);
}
}
void app_RemoteLight(void){
  TS_Point p = ts.getPoint();
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
  if(p.z>200){
    digitalWrite(TFT_BL, HIGH);
   if(p.y<48 && p.x>210){app=1;draw_AlarmClock();}
   else if(p.y<one_thirdBOXSIZE){
      IrSender.sendOnkyo(0x1275, 0x207, 0);//ON&OFF
      tft.drawRect(0,40,TFT_WIDTH,66,TFT_BLUE);
      delay(1000);
      tft.drawRect(0,40,TFT_WIDTH,66,TFT_BLACK);
    }
   else if(p.y<212){
    if(p.x<halfWidth){//暗
      IrSender.sendOnkyo(0x1275, 0x206,0);
      tft.drawRect(0,one_thirdBOXSIZE,halfWidth,one_thirdBOXSIZE,TFT_BLUE);
      delay(1000);
      tft.drawRect(0,one_thirdBOXSIZE,halfWidth,one_thirdBOXSIZE,TFT_BLACK);      
    }
    if(p.x>halfWidth){//明
      IrSender.sendOnkyo(0x1275, 0x208, 0);
      tft.drawRect(halfWidth,one_thirdBOXSIZE,halfWidth,one_thirdBOXSIZE,TFT_BLUE);
      delay(1000);
      tft.drawRect(halfWidth,one_thirdBOXSIZE,halfWidth,one_thirdBOXSIZE,TFT_BLACK);      
    }
   }
   else if(p.y<278){//常夜灯
      IrSender.sendOnkyo(0x1275, 0x203, 0);
      tft.drawRect(0,212,TFT_WIDTH,66,TFT_BLUE);
      delay(1000);
      tft.drawRect(0,212,TFT_WIDTH,66,TFT_BLACK);    
   }
        else if(p.y>285){
        digitalWrite(TFT_BL, LOW);
        sleepTimer=50;
      }
}
   delay(200);
}


void setup() {
 dmaBuffer1[16*16];
  Serial.begin(9600);
  ts.begin();
  ts.setRotation(4);
  tft.begin();
  tft.setTextColor(0xFFFF, 0x0000);
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  IrSender.begin(IR_SEND_PIN); 
   TS_Point p = ts.getPoint();
   p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
   p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  tft.setTextDatum(MC_DATUM);
  tft.setSwapBytes(true);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);  
  DrawStartMenu_Dots();
  
  tft.setFreeFont(FF21);
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  tft.println("SPIFFS_ok");
  //spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_SS);
  //if (!SD.begin(SD_SS, spiSD)) {
 //   while (1) delay(0);
  //}
  tft.println("SD_ok");
  if(p.z>200){
  if(p.x>halfWidth){
  ssid = "your ssidA";
  password = "your passwordA";
  tft.println("Select Phone");    
  }
  if(p.x<halfWidth){
 ssid = "your ssidA";
  password = "your passwordA";
  tft.println("Select Home WIFI");  
  }
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    tft.print(".");
    delay(500);
  }
  tft.println("WIfi_ok");
  }
  else if(p.z<200){
    tft.println("Choose No Conecct.");
  }
  
tft.initDMA();
  delay(10);

  tft.fillScreen(TFT_BLACK);
  uint16_t w = 0, h = 0;
  time_t t;
  struct tm *tm;
 // tft.setCursor(50, 100);
  static const char *wd[7] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
  t = time(NULL);
  tm = localtime(&t);
  oldTime= tm->tm_min;
  //draw_RemoteLightGrid();
  tft.startWrite();
  tft.fillScreen(TFT_BLACK);
  TJpgDec.setJpgScale(3);// The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setCallback(tft_output);
  TJpgDec.getSdJpgSize(&w, &h, "/3X4/GogouForest34.jpeg");
  TJpgDec.drawSdJpg(0, 0, "/3X4/GogouForest34.jpeg");
  tft.endWrite();
  draw_RemoteLightGrid();
}
void loop() {
   if (ts.bufferEmpty()) {
     return;
     }
  uint16_t w = 0, h = 0;
  time_t t;
  struct tm *tm;
 // tft.setCursor(50, 100);
  static const char *wd[7] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
  t = time(NULL);
  tm = localtime(&t);
  //int oldTime = tm->tm_min;
// app_RemoteLight();
   TS_Point p = ts.getPoint();
   p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
   p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
   if(p.z>200){digitalWrite(TFT_BL, HIGH);sleepTimer=0;}
    for(;;){
    if(app==1){app_AlarmClock();}
    else if(app==2){app_RemoteLight();}
    else if(app==0){}
  if (tm->tm_min != oldTime){
  tft.startWrite();
  tft.fillRect(0,0,240,40,TFT_BLACK);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
  tft.setCursor(0, 0);
  tft.setTextColor(ShDColor);
  tft.setFreeFont(FF21);
  tft.startWrite();
  tft.println();
  tft.print("    ");
  tft.print(tm->tm_hour);
  tft.print(":");
  if(tm->tm_min<10){tft.print("0");}
  tft.print(tm->tm_min);
  tft.print("    ");
  tft.print(tm->tm_year+1900);
  tft.print(" "); 
  tft.print(tm->tm_mon+1);
  tft.print("/");
  tft.print(tm->tm_mday);
  tft.print("[");
  tft.print(wd[tm->tm_wday]);
  tft.print("]");
  //tft.setFreeFont(FF20);
  tft.endWrite();
  if((tm->tm_hour)==AlarmHour && (tm->tm_min)==AlarmMin && (tm->tm_sec)==0 && IsAlarmON==true){
     digitalWrite(TFT_BL, HIGH);
      IrSender.sendOnkyo(0x1275, 0x207, 0);//ON&OFF
      tft.drawRect(0,0,TFT_WIDTH,one_thirdBOXSIZE,TFT_BLUE);
      delay(1000);
      tft.drawRect(0,0,TFT_WIDTH,one_thirdBOXSIZE,TFT_BLACK);
      for(int i=0; i<5; i++){
        IrSender.sendOnkyo(0x1275, 0x208, 0);//明
      tft.drawRect(halfWidth,one_thirdBOXSIZE,halfWidth,one_thirdBOXSIZE,TFT_BLUE);
      delay(1000);
      tft.drawRect(halfWidth,one_thirdBOXSIZE,halfWidth,one_thirdBOXSIZE,TFT_BLACK);
      }
  }
  oldTime= tm->tm_min;
  sleepTimer++;
  if(sleepTimer>2){digitalWrite(TFT_BL, LOW);sleepTimer=0;delay(200);}
     delay(200);
  }
     delay(20);
  }
}

/*
if (tm->tm_min != oldTime){
  tft.startWrite();
  tft.fillRect(BTSIZE*2,0,BTSIZE,BTSIZE, TFT_GRAY);
  tft.setCursor(90, 10);
  tft.setFreeFont(FF22);
  tft.setTextColor(ShTColor);
  tft.print(tm->tm_hour);
  tft.print(":");
  if(tm->tm_min<10){tft.print("0");}
  tft.println(tm->tm_min);
  tft.setCursor(80, 30);
  tft.setTextColor(ShDColor);
  tft.setFreeFont(FF21);
  tft.startWrite();
  tft.println();
  tft.print(tm->tm_year+1900);
  tft.print(" "); 
  tft.print(tm->tm_mon+1);
  tft.print("/");
  tft.print(tm->tm_mday);
  tft.print("[");
  tft.print(wd[tm->tm_wday]);
  tft.println("]");
  tft.endWrite();
 delay(100);
  oldTime= tm->tm_min;
  }
*/



