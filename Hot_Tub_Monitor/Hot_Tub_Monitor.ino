#include <OneWire.h>
#include <DallasTemperature.h>
#include "EmonLib.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial
double t;
BlynkTimer timer;
void requestTime() {
  Blynk.sendInternal("rtc", "sync");
}
BLYNK_WRITE(InternalPinRTC) {
  t = param.asDouble();  //Unix time from blynk NTP server
}

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     0 // Reset pin # (or -1 if sharing Arduino reset pin)

// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS D3

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
char auth[] = "I6cEXCSWkCGpOx54E05v0P8RBV5ibEOf";
char ssid[] = "TALKTALK580025";
char pass[] = "38E7YE86";

//variables
int Ttub;
int Tamb;
double Irms;
int POWER;
int display_toggle;
double kWh = 0;
float kWhBuffer = 0;
float currentMillis = 1;
float prevMillis = 0;
float cost = 0; //£s
float bufferMillis=0;
float bufferT=0;
float currentT=1;
float CostPerUnit = 12.8; //pence per kWh
float bufferSize = 300000; //ms to hold buffer
int done = 0;   //first loop indicator

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  
EnergyMonitor emon1;


// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

BLYNK_WRITE(V3) { //Display Toggle
  display_toggle = param.asInt(), DEC;
}
BLYNK_WRITE(V5) {
CostPerUnit = param.asDouble();
}
BLYNK_WRITE(V10){  //button to reboot, to reset cost counter. dirty but what you gonna do
  if (param.asInt()==1){
    delay(3000);
    ESP.restart();
    delay(5000);
  }
}

void setup(void)
{
  Serial. begin(9600);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("   Connecting to "));
  display.setCursor(0,10);             // Start at top-left corner
  display.println(F("WIFI - TALKTALK580025"));
  display.setCursor(0,20);             // Start at top-left corner
  display.println(F("Pswd - 38E7YE86"));
  display.display();
  delay(3000);
  
  Blynk.begin(auth, ssid, pass);
  Blynk.syncAll();
  
  testdrawcircle();    // Draw circles (outlines)
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("     Welcome to"));
  display.setTextSize(1.5);             // Normal 1:1 pixel scale
  display.setCursor(0,10);             // Start at top-left corner
  display.println(F("      Hot Tub"));
  display.setCursor(0,20);             // Start at top-left corner
  display.println(F("      Monitor!"));
  display.display();
  delay(3000);
  
  display.display();

  sensors.begin();  // Start up the library
//  Serial.begin(9600);
 
  emon1.current(A0, 111.1);             // Current: input pin, calibration.
  timer.setInterval(1000L, requestTime); //interrupt to update RTC every second

}



void loop(void)
{ 
  Blynk.run();
  timer.run();

  currentT=t;
  
  if(done==0 && t>5){  //execute the first loop after RTC value has updated. 0 value for buffer clock
    bufferT=t;
    bufferMillis=currentMillis;
    kWhBuffer = 0;
    done=1;
    Serial.print("done - buffert= ");
    Serial.println(bufferT);
  }

  Irms = emon1.calcIrms(1480);
  if (Irms < 0.2 || millis() < 25000){
    Irms = 0;
  }
  Irms*=3.22; //multiplier including power factor.


  POWER = 230*Irms;
    prevMillis = currentMillis;
    currentMillis = millis();
  kWhBuffer+=(currentMillis-prevMillis)*POWER*0.001*0.00027778*0.001; //0.001=millis to s, 0.0002778=Ws to WH (1/3600), 0.001=WH to kWh
 
  
  if((currentMillis - bufferMillis) > bufferSize && done==1){  //5 minutes since buffer cleared
    cost+=(CostPerUnit*(kWhBuffer*((t-bufferT)/(currentMillis-bufferMillis)))*0.01*1000); //correction for millis drift over 5 mins. 0.01= #s to p, 1000 = seconds to milliseconds
    kWh+=((kWhBuffer*((t-bufferT)/(currentMillis-bufferMillis)))*1000);
     
    bufferT=t;
    bufferMillis = currentMillis;  //reset buffer timers
    kWhBuffer = 0; 
  }

   
  
  // Send the command to get temperatures
  sensors.requestTemperatures(); 

  Ttub = sensors.getTempCByIndex(0);
  Tamb = sensors.getTempCByIndex(1);

  if (Tamb < -100){Tamb = 0.0;}
  if (Ttub < -100){Ttub = 0.0;}

  Blynk.virtualWrite(V0, Ttub); //sending to 
  Blynk.virtualWrite(V1, Tamb); //sending to Blynk
  Blynk.virtualWrite(V2, Irms); //sending to Blynk
  Blynk.virtualWrite(V4, POWER); //sending to Blynk
  Blynk.virtualWrite(V6, cost + (CostPerUnit*0.01*kWhBuffer));  //includes buffered estimate, which will be scrubbed when time is updated
  Blynk.virtualWrite(V7, kWh + kWhBuffer);

  display.clearDisplay();

if (display_toggle == 1){

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("Power = "));
  display.print(POWER);
  display.println(" W");


  display.print("WaterTemp = ");
  display.print(Ttub);
  display.println(" C");


  display.print("AmbientTemp = ");
  display.print(Tamb);
  display.println(" C");


  display.print(kWh+kWhBuffer,2);
  display.print(" kWh   $ ");
  display.print(cost+(kWhBuffer*CostPerUnit*0.01),2);
  

  display.display();

}
  

if (display_toggle == 0){

  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("               "));
  display.println(F("  OFF"));
  display.display();
}
}

void testdrawcircle(void) {
  display.clearDisplay();

  for(int16_t i=0; i<max(display.width(),display.height()); i+=1) {
    display.drawCircle(display.width()/2, display.height()/2, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  
  delay(1000);
}
