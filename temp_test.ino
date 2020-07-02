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

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

  
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "I6cEXCSWkCGpOx54E05v0P8RBV5ibEOf";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "15A2";
char pass[] = "Edinburgh123";


//variables
float Ttub;
float Tamb;
double Irms;

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  
EnergyMonitor emon1;


// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);


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
  display.println(F("   Connecting..."));
  display.display();
  delay(3000);
  testdrawcircle();    // Draw circles (outlines)
  sensors.begin();  // Start up the library
//  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  emon1.current(A0, 111.1);             // Current: input pin, calibration.


}



void loop(void)
{ 
  Blynk.run();

  Irms = emon1.calcIrms(1480);

  // Send the command to get temperatures
  sensors.requestTemperatures(); 

  Ttub = sensors.getTempCByIndex(0);
  Tamb = sensors.getTempCByIndex(1);

  if (Tamb < -100){Tamb = 0.0;}
  if (Ttub < -100){Ttub = 0.0;}

  Blynk.virtualWrite(V0, Ttub); //sending to 
  Blynk.virtualWrite(V1, Tamb); //sending to Blynk
  Blynk.virtualWrite(V2, Irms); //sending to Blynk

  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("   Hot Tub Monitor"));

  display.print("WaterTemp = ");
  display.println(Ttub);

  display.print("AmbientTemp = ");
  display.println(Tamb);
  
  display.print("Current = ");
  display.print(Irms);
  display.println(" Amps");

  display.display();
  delay(2000);

}

void testdrawcircle(void) {
  display.clearDisplay();

  for(int16_t i=0; i<max(display.width(),display.height()); i+=1) {
    display.drawCircle(display.width()/2, display.height()/2, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(2000);
}
