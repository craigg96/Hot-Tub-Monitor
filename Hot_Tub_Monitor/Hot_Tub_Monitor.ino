#include <OneWire.h>
#include <DallasTemperature.h>
#include "EmonLib.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     0 // Reset pin # (or -1 if sharing Arduino reset pin)

// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS D3

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

  
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "I6cEXCSWkCGpOx54E05v0P8RBV5ibEOf";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "TALKTALK580025";
char pass[] = "38E7YE86";

//variables
int Ttub;
int Tamb;
double Irms;
int POWER;
int display_toggle;

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  
EnergyMonitor emon1;


// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

BLYNK_WRITE(V3) { //Display Toggle
  display_toggle = param.asInt(), DEC;
}


void setup(void)
{
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  // display.display() is NOT necessary after every single drawing command,
  // unless that's what you want...rather, you can batch up a bunch of
  // drawing operations and then update the screen all at once by calling
  // display.display(). These examples demonstrate both approaches...

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


}



void loop(void)
{ 
  Blynk.run();

  Irms = emon1.calcIrms(1480);

  if (Irms < 0.5){
    Irms = 0;
  }
  
  if (millis() < 12000){
    Irms = 0;
  }

  POWER = 230*Irms;
  
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

  display.clearDisplay();
  display.display();

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

  
  display.print("Current = ");
  display.print(Irms);
  display.println(" Amps");

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
