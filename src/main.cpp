
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

char appName[7][12]={"Home","AlarmClock","RemoteLight","Paint","Line","Gyanken","Settings"};

uint16_t ShTColor = TFT_WHITE;
uint16_t ShDColor = TFT_WHITE;
  int sleepTimer=0;
  int app=0;
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
void draw_Header(void){
  tft.fillRect(0,0,240,40,TFT_BLACK);
}
void draw_HomeMenu(void){
tft.fillRect(0,40,240,320-40,TFT_BLACK);

tft.drawRoundRect(240-40,100, 40,80, 10,TFT_BLUE);//上
tft.drawRoundRect(240-40,180, 40,40, 10,TFT_BLUE);//決定
tft.drawRoundRect(240-40,220, 40,80, 10,TFT_BLUE);//下
tft.drawRoundRect(240-40,180, 40,40, 10,TFT_CYAN);//決定ボタン装飾
tft.drawRoundRect(240-40+5,180+5, 30,30, 7,TFT_CYAN);//決定ボタン装飾
tft.drawLine(220,132,210,148,TFT_CYAN);// 上矢印
tft.drawLine(220,132,230,148,TFT_CYAN);//上矢印
tft.drawLine(220,265,210,250,TFT_CYAN);//下矢印
tft.drawLine(220,265,230,250,TFT_CYAN);//下矢印
  for(int i=2; i<8; i++){//40*7
    tft.drawRect(12,i*40,182,40,TFT_DARKCYAN);
  }
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(FF21);
  for(int i=1; i<7; i++){
    tft.drawCentreString((String)appName[i],101,i*40+50,2);
  }
}
void draw_RemoteLightGrid(void){
  #define BTSIZE 80
  tft.fillRect(0,40,240,320-40,TFT_BLACK);
  tft.fillRoundRect(0,40,TFT_WIDTH,66,10,TFT_GRAY);
  tft.fillRoundRect(0,106,120,106,10,TFT_GRAY);
  tft.fillRoundRect(120,106,120,106,10,TFT_GRAY);
  tft.fillRoundRect(0,212,TFT_WIDTH,66,10,TFT_GRAY);
  tft.fillCircle(220,15,10,TFT_GREENYELLOW);
  tft.drawRect(0,40,TFT_WIDTH,66,TFT_BLACK);
  tft.drawRect(0,106,120,106,TFT_BLACK);
  tft.drawRect(120,106,120,106,TFT_BLACK);
  tft.drawRect(0,212,TFT_WIDTH,66,TFT_BLACK);
  tft.setTextColor(TFT_BLUE);
  tft.setFreeFont(FF21);
  tft.drawCentreString("ON/OFF",120,73,2);
  tft.drawCentreString("Night",120,245,2);
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
  if(IsAlarmON==false){tft.drawRoundRect(195,245,40,30,10,TFT_GREEN);}
   tft.setTextColor(TFT_BLUE,TFT_BLACK);
       tft.drawCentreString((String)AlarmHour,60,120,1);
       tft.drawCentreString(":",120,120,1);
       tft.drawCentreString((String)AlarmMin,180,120,1);
        tft.drawRoundRect(0,285,240,35,2,TFT_DARKCYAN);
        tft.setTextColor(TFT_WHITE,TFT_BLACK);
        tft.drawCentreString("Sleep",115,297,2);
        tft.setTextColor(TFT_BLUE);
        tft.setCursor(0,0);
}
#define TouchDelayHome 100
void app_HomeMenu(void){
   TS_Point p = ts.getPoint();
   p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
   p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
   if(p.z>200){
    if(p.x>200){
      if(p.y>80 && p.y<160){//上
      tft.fillRoundRect(240-40,100, 40,80, 10,TFT_BLUE);//上
      delay(TouchDelayHome);
      tft.fillRoundRect(240-40,100, 40,80, 10,TFT_BLACK);
      tft.drawRoundRect(240-40,100, 40,80, 10,TFT_BLUE);
      tft.drawLine(220,132,210,148,TFT_CYAN);
      tft.drawLine(220,132,230,148,TFT_CYAN);
      } 
      if(p.y>160 && p.y<200){//決定
      tft.fillRoundRect(240-40+5,180+5, 30,30, 7,TFT_CYAN);//決定
      delay(TouchDelayHome);
      tft.fillRoundRect(240-40+5,180+5, 30,30, 7,TFT_BLACK);
      } 
      if(p.y>200 && p.y<280){//下
      tft.fillRoundRect(240-40,220, 40,80, 10,TFT_BLUE);//下
      delay(TouchDelayHome);
      tft.fillRoundRect(240-40,220, 40,80, 10,TFT_BLUE);
      tft.drawRoundRect(240-40,220, 40,80, 10,TFT_BLUE);
      tft.drawLine(220,265,210,250,TFT_CYAN);//下矢印
      tft.drawLine(220,265,230,250,TFT_CYAN);//下矢印
      } 
    }
    if(p.x<200){
      if(p.y>80){
        if(p.y<120){
           tft.drawRect(12,80,182,40,TFT_RED);
           delay(TouchDelayHome);
          app=1;
          draw_AlarmClock();
        }
        if(p.x<160 && p.y>120){
           tft.drawRect(12,120,182,40,TFT_RED);
           delay(TouchDelayHome);
          app=2;
          draw_RemoteLightGrid();
        }
        if(p.x<200 && p.x>160){}
        if(p.x<240 && p.x>200){}
        if(p.x<280 && p.x>240){}
        if(p.x<320 && p.x>280){}
      }
    }
   }
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
   if(p.y<40 && p.x>200){app=0;draw_HomeMenu();delay(500);app_HomeMenu();}//閉じる
    if(p.y<120 && p.y>40){//up
      if(p.x<120){//時間up
        AlarmHour=AlarmHour+1;
        if(AlarmHour>23){AlarmHour=0;}
        tft.fillRect(20,115,90,60,TFT_BLACK);
        tft.drawCentreString((String)AlarmHour,60,120,1);
         delay(100);
        }
      if(p.x>120){//分up
        AlarmMin=AlarmMin+1;
        if(AlarmMin>60){AlarmMin=0;}
        tft.fillRect(140,115,90,60,TFT_BLACK);
        tft.drawCentreString((String)AlarmMin,180,120,1);
         delay(60);
        }

    }
    else if(p.y<240){//down
      if(p.x<120){//時間down
        AlarmHour=AlarmHour-1;
        if(AlarmHour<0){AlarmHour=23;}
        tft.fillRect(20,115,90,60,TFT_BLACK);
        tft.drawCentreString((String)AlarmHour,60,120,1);
         delay(100);
        }
      if(p.x>120){//分down
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
  if(p.y<40 && p.x>200){app=0;draw_HomeMenu();delay(500);app_HomeMenu();}//閉じる
   if(p.y>40){
   if(p.y<106){
      IrSender.sendOnkyo(0x1275, 0x207, 0);//ON&OFF
      tft.drawRect(0,40,TFT_WIDTH,66,TFT_BLUE);
      delay(1000);
      tft.drawRect(0,40,TFT_WIDTH,66,TFT_BLACK);
    }
   else if(p.y<212){
    if(p.x<120){//暗
      IrSender.sendOnkyo(0x1275, 0x206,0);
      tft.drawRect(0,106,120,106,TFT_BLUE);
      delay(1000);
      tft.drawRect(0,106,120,106,TFT_BLACK);      
    }
    if(p.x>120){//明
      IrSender.sendOnkyo(0x1275, 0x208, 0);
      tft.drawRect(120,106,120,106,TFT_BLUE);
      delay(1000);
      tft.drawRect(120,106,120,106,TFT_BLACK);      
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
  if(p.x>120){
  ssid = "tellurnoidWifi";
  password = "00000000";
  tft.println("Select Phone");    
  }
  if(p.x<120){
  ssid = "A0957F978755-5G";
  password = "ftn4y7hen4sr6h";  
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
  draw_Header();
  draw_HomeMenu();
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
   for(;;){
   if(p.z>200){digitalWrite(TFT_BL, HIGH);sleepTimer=0;}
    if(app==1){app_AlarmClock();}
    else if(app==2){app_RemoteLight();}
    else if(app==0){app_HomeMenu();}
  if (tm->tm_min != oldTime){
  tft.startWrite();
  tft.fillRect(0,0,210,40,TFT_BLACK);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
  tft.setCursor(0, 0);
  tft.setTextColor(ShDColor);
  tft.setFreeFont(FF21);
  tft.startWrite();
  tft.println();
  tft.print("  ");
  tft.print(tm->tm_hour);
  tft.print(":");
  if(tm->tm_min<10){tft.print("0");}
  tft.print(tm->tm_min);
  tft.print("    ");
  //tft.print(tm->tm_year+1900);
  //tft.print(" "); 
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
   IrSender.sendOnkyo(0x1275, 0x202, 0);
   delay(1000);
   sleepTimer=0;
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



