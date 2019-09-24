/*
  PcAlphanumerics
  Kenny Kim
  April 2019

  This program is supposed to control a set of 14-segment displays all lined up. Each display has 4 digits.
  I know this is inefficient as heck but it's what I'm going for now.
*/

// ===IMPORTS ===
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"



// === CONSTANTS ===
const Adafruit_AlphaNum4 displaySeg[] = {Adafruit_AlphaNum4(), Adafruit_AlphaNum4(), Adafruit_AlphaNum4(), Adafruit_AlphaNum4()};
const int displayAddresses[] = {0x71, 0x72, 0x73, 0x74};
const int numberOfDisplays = 4;
char displayBuffer[numberOfDisplays*4];
int timeout = 200;



// === HELPER FUNCTIONS ===
//addSpaces adds a number of spaces to the beginning or end of given string
String addSpaces(String text, int number, bool beginning){
  if (beginning){for(int i=0; i<number; i++) text = " " + text;}
  else{for(int i=0; i<number; i++) text+=" ";}
  return text;
}

//scrollPrint scrolls the text from left to write, updating every 0.2 seconds
// erase concats spaces so it scrolls off the display
void scrollPrint(String text, bool erase){
  clearDisplay();
  if(erase){ //add spaces at the end if erase is true
    text = addSpaces(text, numberOfDisplays*4, false);
  }
  for(int i=0; i<text.length(); i++){
    
    char c = text.charAt(i);
    
    if(!isprint(c)) continue;
    
    
    for(int ii=0; ii<15; ii++){
      displayBuffer[ii] = displayBuffer[ii+1];
      displaySeg[ii/4].writeDigitAscii(ii%4, displayBuffer[ii]);
    }
    
    displayBuffer[(4*numberOfDisplays)-1] = c;
    displaySeg[numberOfDisplays-1].writeDigitAscii(3, c);
    
    for(int ii=0; ii<numberOfDisplays; ii++){
      displaySeg[ii].writeDisplay();
    }

    delay(timeout);
  }
}


//staticPrint prints up to 16 characters of the text without scrolling
// align: 0 = left, 1 = center, 2 = right
void staticPrint(String text, int align){
  clearDisplay();
  int length = min(text.length(), numberOfDisplays*4);
  
  if(align == 0){ //left align
    for(int i=0; i<length; i++){
      displaySeg[i/4].writeDigitAscii(i%4, text.charAt(i));
    }
    for(int i=0; i<4; i++){
      displaySeg[i].writeDisplay();
    }
  }

  if(align == 1){ //center align
    int spaces = (16-length)/2;
    staticPrint(addSpaces(text, spaces, true), 0);
  }

  if(align == 2){ //right align
    int spaces = 16-length;
    staticPrint(addSpaces(text, spaces, true), 0);
  }
}


//clearDisplay replaces everything with spaces, probably used after static display
void clearDisplay(){
  for(int i=0; i<numberOfDisplays; i++){
    for(int ii=0; ii<4; ii++){
      displaySeg[i].writeDigitAscii(ii, ' ');
    }
    displaySeg[i].writeDisplay();
  }
}


//statusReport gives a simple report on connected displays
void statusReport(){
  Serial.println("numberOfDisplays:"+String(numberOfDisplays));
  String addresses = "";
  for(int i=0; i<numberOfDisplays; i++){
    addresses += String(displayAddresses[i], HEX) + ",";
  }
  addresses.remove(addresses.length()-1);
  Serial.println("addresses:"+addresses);
}



// === MAIN FUNCTIONS ===
// =Setup=
void setup() {
  Serial.begin(9600);
  Serial.println("Starting Alphanumerics");
  for(int i=0; i<numberOfDisplays; i++){
    Serial.println(i);
    displaySeg[i].begin(displayAddresses[i]);
  }
  for(int i=0; i<numberOfDisplays*4; i++){
    displayBuffer[i] = ' ';
  }
}

// =Loop=
void loop() {
  while(!Serial.available()) return; //check to see if there is any serial at all
  int arg = 0;
  
  String message = Serial.readStringUntil('\n');
  switch(message.charAt(0)){
    case '0': //Clear Display
      clearDisplay();
      break;
    case '1': //Scroll Print
      scrollPrint(message.substring(2), message.substring(1, 2).toInt());
      break;
    case '2': //Static Print
      staticPrint(message.substring(2), message.substring(1, 2).toInt());
      break;
    case '3': //Status Report
      statusReport();
      break;
    case '4': //Change Timeout
      timeout = message.substring(1).toInt();
      Serial.println("New timeout: "+String(timeout));
    default:
      return;
  }
}
