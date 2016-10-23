/**************************************************************
 * DESCRIPTION 
 * Internet_clock is a program to get time via NTP and tempature
 * from Weather Underground
 *
 * Adafruit Huzzah ESP8266 with: 
 * Adafruit OLED Monochrome 1.3" 128x64 display (Product ID: 938) 
 *    
 * Created by Mike Pate <k5map@arrl.net>
 * Copyright (C) 2016 by A Third Opinion, LLC
 * Repository: https://github.com/k5map/PiPower_HAT/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * REVISION HISTORY
 * Version 1.0 - initial release
 */
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>                      //Serial Peripheral Interface (SPI) library for synchronous serial data protocol
#include <Wire.h>                     //Wire library used for I2C communication: Arduino Pro Mini pins used = A4 (SDA) and A5 (SCL)
#include <Adafruit_GFX.h>             //Adafruit graphic display library used for the OLED display
#include <Adafruit_SSD1306.h>         //Adafruit driver for OLED

#define OLED_RESET 14      // GPIO pin
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 64)
  #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
time_t getNtpTime();

int prevSecond = 0;
String colonString = " ";
String minuteString = " ";
int CurrentTemp = 0;

//***  Weather Underground parms
const char server[] = "api.wunderground.com";    // name address for Weather Underground (using DNS)
const String myKey = "xxxxxxxxxxxxxxxx";  //See: http://www.wunderground.com/weather/api/d/docs (change here with your KEY)
const String myFeatures = "conditions";   //See: http://www.wunderground.com/weather/api/d/docs?d=data/index&MR=1
const String myZipcode = "77379";

const int WU_INTERVAL = 900;        // number of secs to refresh weather data
String html_cmd1 = "GET /api/" + myKey + "/" + myFeatures + "/q/" + myZipcode + ".json HTTP/1.1";
String html_cmd2 = "Host: " + (String)server;
String html_cmd3 = "Connection: close";
String responseString;
int prevCheck = 0;    
boolean startCapture;

void digitalClockDisplay();

WiFiClient client;


/**************** Setup **********************************************/
void setup()   {                  
  Serial.begin(115200);
  delay(10);
  Serial.println();  Serial.println();
  Serial.println("Feather booting up...\n");
  Serial.println("ESP8266:");
  Serial.printf("- Chip-ID = %08X\n", ESP.getChipId());
  Serial.printf("- Heap Size = %i\n", ESP.getFreeHeap());
  Serial.printf("- Flash Chip-ID = %i\n", ESP.getFlashChipId());
  Serial.printf("- Flash Chip Size = %i\n", ESP.getFlashChipSize());
  Serial.printf("- Flash Chip Speed = %i\n", ESP.getFlashChipSpeed());
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize the OLED and set the I2C address to 0x3C (for the 128x64 OLED) 
  
  display.clearDisplay();             //Clear display buffer.
  display.setTextSize(2);             //set text size to 2 (large)
  display.setTextColor(WHITE);
  display.setCursor(0,0);             //set text start position to column=0 and row=0
  display.print("K5MAP");
  
  display.setTextSize(1);             //set text size to 1 (small)
  display.setCursor(0,18);            //set text start position to column=0 and row=18
  display.print("INTERNET CLOCK");
  
  display.setCursor(0,30);              //set text start position to column=0 and row=30
  display.setTextColor(BLACK, WHITE);   // 'inverted' text
  display.print("INITIALISING .....");  //print "INITIALISING ....." to display
  
  display.setCursor(0,40);            //set text start position to column=0 and row=40
  display.setTextColor(WHITE);
  display.print("Attempting to connect to WiFi network");
  
  display.setCursor(0,57);            //set text start position to column=0 and row=57
  display.print("Please Wait...");    //print "Please Wait" to display
  
  display.display();                  //update OLED with new display data
  delay(1000);                        //short delay
  display.clearDisplay();             //clear display
    
  // Connect to a WiFi network
  connectWiFi();
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(600);
  setDST();
  setSyncProvider(getNtpTime);

  prevSecond = now();
}


/**************** Main Loop **********************************************/
void loop() {
  // check for change in seconds
  if ((now() - prevSecond) >= 1) {
    prevSecond = now();

    colonString = (colonString == " ") ? ":" : " ";
    minuteString = (minute()<10) ? "0" + String(minute()) : String(minute());

    digitalClockDisplay();  
  }
  if (hour() == 2)  setDST();

  // check to see if weather data needs to be refreshed
  if ((now() - prevCheck) >= WU_INTERVAL)  {
    sendWeatherGetData();
    prevCheck = now();
  }

  if (client.available())  {
    char c = client.read();
    if (c == '{')
      startCapture=true;
    
    if (startCapture)
      responseString += c;
  }

  if (!client.connected())  {
    client.stop();
    client.flush();
        
    String TempString = getValuesFromKey(responseString, "temp_f");
    Serial.println("TempString = " + TempString);
    CurrentTemp = TempString.toInt();
    Serial.println(CurrentTemp);
  }
}
 
 
/**************************************************/
void digitalClockDisplay()  {  
  display.clearDisplay();
  
  display.setTextSize(1);           //set text size to 1
  display.setTextColor(WHITE);      
  display.setCursor(50,0);          //set text start position for date and time (row = 0, column =0)
  display.print(String(monthShortStr(month())) + " " + String(day()));    // prints date on top line of OLED
  
  display.setTextSize(2);           //set text size to 2
  if (hourFormat12() < 10) {
    display.setCursor(40,25);         //set text start position to column=0 and row=20
  }
  else {
    display.setCursor(32,25);         //set text start position to column=0 and row=20
  }
  display.print(hourFormat12() + colonString + minuteString);   //prints time to OLED
  display.setTextSize(1);
  display.print(isAM() ? " AM" : " PM");
  
  display.setTextSize(1);           //set text size to 1
  display.setCursor(20,55);         //set text start position to column=0 and row=20
  display.print("Outside Temp: " + String(CurrentTemp) + (char)247);   //prints temp to OLED

  display.display();                //update OLED display
}


/**************************************************/
void sendWeatherGetData()  {
  responseString = "";
  if (!client.connect(server, 80))  {
    Serial.println("Connection to Weather Underground failed...");
    return;
  }

  // Make a HTTP request:
  client.println(html_cmd1);
  client.println(html_cmd2);
  client.println(html_cmd3);
  client.println();
  startCapture = false;   
}

