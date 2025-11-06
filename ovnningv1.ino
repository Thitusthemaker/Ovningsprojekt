/*
* Name: övningsprojekt
* Author: Thitus Lundgren
* Date: 2025-11-05
* Description: This project uses a ds3231 to measure time and displays the time to an 1306 oled display simulating an analog clock, 
* Further, it measures temprature and displays a mapped value to a 9g-servo-motor.
* It also shows the seconds in a resolution of 2,5 seconds and the temperature (green to yellow to red) on an neopixel.
*/

// Include Libraries
#include <RTClib.h>
#include <Wire.h>
#include "U8glib.h"
#include <Servo.h>
#include <math.h>
#include <Adafruit_NeoPixel.h>

// Init constants

#define NUMPIXELS 24  // Pixels on the neopixel
#define ledPin 6  // The digital pin connected to the neopixel
#define beep 3 // The PWM pin connected to the piezo

const float tempMin = 18.00;  //Temperature min mapped value
const float tempMax = 25.00;  //Temperature max mapped value

// Init global variables

int oldmin = 0;
// Construct objects
RTC_DS3231 rtc;
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);
Adafruit_NeoPixel pixels(NUMPIXELS, ledPin, NEO_GRB + NEO_KHZ800);
Servo spinner;

void setup() {
  // init communication
  Serial.begin(115200);
  Wire.begin();   
  

  // Init Hardware
  rtc.begin();  // INITIALIZE rtc library
  spinner.attach(9); // ATTACH servo
  pixels.begin(); // INITIALIZE NeoPixel strip object

  // Settings
  pinMode(A0,0);

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // UPPLOADS date and time to ds3231
  spinner.write(0); // Zeros the servo

}
/*
* Do you really not know what this is supposed to do? Git gud
** Parameters: void
** Returns: no
*/
void loop() {

  servoWrite(getTemp()); // Writes the mapped temperature on the servo

  oledWrite(clockhands("xh"),clockhands("yh"),clockhands("xm"),clockhands("ym")); // Displays the analog hour and minute clock hands on the display.
  ledlights(getSeconds(), getTemp()); // Writes how many lights and which colours should be on based on the seconds and temperature.
  sounds(); //Checks if it is a new minute and if it is it will send a tone to a piezo.

  delay(20);
}
/*
*This functions sends a tone to a piezo every minute.
*Parameters: void
*Returns: nothing
*/
void sounds(){
  DateTime now = rtc.now();
  if(now.minute() != oldmin){ // Checks if it is a new minute by comparing it to "oldmin" if it is oldmin will become the current minute and it will send a tone.
    oldmin = now.minute();
    tone(beep,300,100);
  }
}


/*
*This function reads time from an ds3231 module and package the time as a String
*Parameters: Void
*Returns: time in hh:mm:ss as a String
*/
String getTime() {
  DateTime now = rtc.now(); // Calls the now() function in the rtc class and saves the time/date in DateTime now variable 
  return String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()); // Returns the hour, minute and seconds in hh:mm:ss as a String
}

/*
* This function takes temprature from ds3231 and returns as a float
* Parameters: Void
* Returns: - float: temperature
*/
float getTemp() {
  return rtc.getTemperature(); // Returns the temperature, what more do i need to say?
}
/*
*This function returns the current seconds.
*Parameters: void
*Returns: - int seconds
*/
int getSeconds(){
  DateTime now = rtc.now();
  return now.second(); //returns the seconds from rtc.now(), what more do i need to say?
}

/*
* This function sends back the x or y coordinates on the screen of the minute hand or hour minute hand depending on the parameter.
* Parameters: - String: xh, yh, xm, ym
* Returns: - int: x or y coordinates based on you parameter 
*/
int clockhands(String what) {
  //Gets the current time
  DateTime now = rtc.now();
  int hour = int(now.hour());
  int min = int(now.minute());
  
  
  // MethemTICS, maps the hour and minutes digital time into analog time with radian degress by multiplies them by 360 degress in radians (2PI) and divides them by the format in analog time.
  float hhand = ((hour % 12)*(2*M_PI))/12; 
  float mhand = ((min % 60)*(2*M_PI))/60;

  // Times the hypotenusa by either sin or cos of the hour and minute in radian to get the coordinates
  float xhhand = 20 * sin(hhand);
  float yhhand = -20 * cos(hhand);

  float xmhand = 30 * sin(mhand);
  float ymhand = -30 * cos(mhand);

  // Returns the centerd value of the x and y coordinates of the hour and minute hands deppending on what was requested.
  if (what == "xh"){
    return int(xhhand) +64;
  }
  if (what == "yh"){
    return int(yhhand)+ (32);
  }
  if (what == "xm"){
    return int(xmhand) + (64);
    
  }
  if (what == "ym"){
    return int(ymhand) + (32);
    
  }

  
}


/*
* This function takes a string and draws it to an oled display.
* Parameters: x and y cords for hour and minute hand. Format: int: xh, int: yh, int: xm, int: ym
* Returns: nothing
*/
void oledWrite(int xh,int yh,int xm,int ym) {
  u8g.setFont(u8g_font_unifont);
  

  
  u8g.firstPage();
  do {

    u8g.drawCircle(64,32, 30); // Draws the circle
    u8g.drawLine(64,32,xh,yh); // Draws the hour hand
    u8g.drawLine(64,32,xm,ym); // Draws the minute hand


  } while (u8g.nextPage());
}

/*
* This function takes a temprature value and maps it to the coresponding degree on a servo
* Parameters: - float: temprature
* Returns: void
*/
void servoWrite(float value) {
  int dir = map(value,tempMin,tempMax,0,180); // Maps the temperature based on tempMin and tempMax to the servos motion range 0-180 degress
  
  
  spinner.write(dir); 
}
/*
* This function writes to the neopixel which lights should be on or off and which color they should display. (Green --> Yellow --> Red)
* Parameters: - int: seconds  - int: temperature
* Returns: nothing
*/
void ledlights(int seconds, int temp){
  pixels.clear(); // Turns of all the pixel colours
  int lights = seconds * NUMPIXELS / 60; // maps the seconds to the number of pixels, with 24 pixels this gives a resolution of 2.5 seconds 
  temp = map(temp,tempMin,tempMax,0,255); // maps the temperature between tempMin and tempMax to the 0-255 color brightness
  
for(int i = 0; i<=lights;i++){    // goes through all the lights which should be lit and set their pixel colour to r = temp; g = (255-temp); this will give this colour gradient  green --> yellow --> red
    pixels.setPixelColor(i, pixels.Color(temp, (255-temp), 0));
  }
  pixels.show(); // Displays the pixel changes.
}