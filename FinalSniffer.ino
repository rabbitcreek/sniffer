
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerifBoldItalic24pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#include <Adafruit_NeoPixel.h>

#define PIN 18
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);

long lastMsg = 0;
char msg[50];
bool HPMAstatus = false;
int zedLevel = 1;
int PM25;
int PM10;
int oldP;
HardwareSerial HPMA115S0(1);
#define RXD2 16
#define TXD2 17
void setup()
{
  Serial.begin(9600, SERIAL_8N1);
  
  delay(100);
  while (!Serial);
   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }
  delay(2000);
strip.begin();
strip.show(); // initialize all pixels to "off"
HPMA115S0.begin(9600, SERIAL_8N1, RXD2, TXD2);
  while (!HPMA115S0);
  start_autosend();
}


void loop()
{
  strip.show();
  brighten(zedLevel);
  long now = millis();
 
  if (now - lastMsg > 1000) {
    lastMsg = now;
    display.clearDisplay();
    // Receive the particle data
    HPMAstatus = receive_measurement();
    if (!HPMAstatus) {
      
      Serial.println("Cannot receive data from HPMA115S0!");
      return;
    }
    snprintf (msg, 16, "%D", PM25);
    
    snprintf (msg, 16, "%D", PM10);
    if(PM10 != 0){
  
    Serial.println("PM 2.5:\t" + String(PM25) + " ug/m3");
    Serial.println("PM 10:\t" + String(PM10) + " ug/m3");
    Serial.println("MQTT published HPMA115S0.");
   
   
  display.setFont(&FreeSerifBold9pt7b);
  display.clearDisplay();
  display.setTextSize(1);             
  display.setTextColor(WHITE);        
  display.setCursor(10,13);             
  display.print("PPM");
  display.setCursor(90,13); 
  display.println("2.5");
  //display.display();
  display.setFont(&FreeSerifBoldItalic24pt7b);
  //display.clearDisplay();
  display.setTextSize(1);             
  display.setTextColor(WHITE);        
  display.setCursor(30,50);             
  display.println(String(PM25));
  display.display();
   if(PM25 < 25) zedLevel = 3;
   else if(PM25 > 24 && PM25 < 80) zedLevel = 2;
  else zedLevel = 1;
 Serial.println(zedLevel);
    }  
  }
}
 
bool receive_measurement (void)
{
  while(HPMA115S0.available() < 32);
  byte HEAD0 = HPMA115S0.read();
  byte HEAD1 = HPMA115S0.read();
  while (HEAD0 != 0x42) {
    if (HEAD1 == 0x42) {
      HEAD0 = HEAD1;
      HEAD1 = HPMA115S0.read();
    } else {
      HEAD0 = HPMA115S0.read();
      HEAD1 = HPMA115S0.read();
    }
  }
  if (HEAD0 == 0x42 && HEAD1 == 0x4D) {
    byte LENH = HPMA115S0.read();
    byte LENL = HPMA115S0.read();
    byte Data0H = HPMA115S0.read();
    byte Data0L = HPMA115S0.read();
    byte Data1H = HPMA115S0.read();
    byte Data1L = HPMA115S0.read();
    byte Data2H = HPMA115S0.read();
    byte Data2L = HPMA115S0.read();
    byte Data3H = HPMA115S0.read();
    byte Data3L = HPMA115S0.read();
    byte Data4H = HPMA115S0.read();
    byte Data4L = HPMA115S0.read();
    byte Data5H = HPMA115S0.read();
    byte Data5L = HPMA115S0.read();
    byte Data6H = HPMA115S0.read();
    byte Data6L = HPMA115S0.read();
    byte Data7H = HPMA115S0.read();
    byte Data7L = HPMA115S0.read();
    byte Data8H = HPMA115S0.read();
    byte Data8L = HPMA115S0.read();
    byte Data9H = HPMA115S0.read();
    byte Data9L = HPMA115S0.read();
    byte Data10H = HPMA115S0.read();
    byte Data10L = HPMA115S0.read();
    byte Data11H = HPMA115S0.read();
    byte Data11L = HPMA115S0.read();
    byte Data12H = HPMA115S0.read();
    byte Data12L = HPMA115S0.read();
    byte CheckSumH = HPMA115S0.read();
    byte CheckSumL = HPMA115S0.read();
    if (((HEAD0 + HEAD1 + LENH + LENL + Data0H + Data0L + Data1H + Data1L + Data2H + Data2L + Data3H + Data3L + Data4H + Data4L + Data5H + Data5L + Data6H + Data6L + Data7H + Data7L + Data8H + Data8L + Data9H + Data9L + Data10H + Data10L + Data11H + Data11L + Data12H + Data12L) % 256) != CheckSumL){
      Serial.println("Checksum fail");
      return 0;
    }
    PM25 = (Data1H * 256) + Data1L;
    PM10 = (Data2H * 256) + Data2L;
    return 1;
  }
}
 
bool start_autosend(void)
{
 // Start auto send
  byte start_autosend[] = {0x68, 0x01, 0x40, 0x57 };
  HPMA115S0.write(start_autosend, sizeof(start_autosend));
  HPMA115S0.flush();
  delay(500);
  //Then we wait for the response
  while(HPMA115S0.available() < 2);
  byte read1 = HPMA115S0.read();
  byte read2 = HPMA115S0.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}
void brighten(int q) {
  uint16_t i, j;
  Serial.print("this is q");
  Serial.println(q);
  switch (q) {
  case 1:
    for (j = 5; j < 155; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, j,  0, 0);
    }
    strip.show();
    delay(255/(j+1));
  }

  
  

  for (j = 155; j > 6; j--) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, j, 0, 0);
    }
    strip.show();
    delay(255/(j+1));
    
  }
    break;
  case 2:
    for (j = 5; j < 155; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 0,  j, 0);
    }
    strip.show();
    delay(255/(j+1));
  }

  
  

  for (j = 155; j > 6; j--) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 0, j, 0);
    }
    strip.show();
    delay(255/(j+1));
    
  }
    break;
  case 3:
   for (j = 5; j < 155; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 0,  0, j);
    }
    strip.show();
    delay(255/(j+1)); 'p4h m  
  }

  
  

  for (j = 155; j > 6; j--) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 0, 0, j);
    }
    strip.show();
    delay(255/(j+1));
    
  }
    break;



 
}
}
