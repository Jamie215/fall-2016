/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

/* 
* Modified by UWHIT Fall 2016: Reminder tool for walking aids
* How this code works:
* Use the delay between the call and response time of 2 RF tranceivers to calculate running avg
* Use delay time to interpret distance
* Alert the user when the response time gets too long
TO DO:
* More robust mechanism to detect distance from the user; maybe use the # of failed pinging
* On/Off switch for the device
* Alert user functions requires more alerting mechanism (i.e. LED, vibration, sound)
*/
#include <QueueList.h>
#include <SPI.h>
#include "RF24.h"

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
/**********************************************************/

const int alertLED = 9;
//const int alertSpeaker = 4;
// dnd: Do Not Disturb mode
//const int dndLED = 5;

byte addresses[][6] = {"1Node","2Node"};

// Used to control whether this node is sending or receiving
bool role = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  radio.begin();
  pinMode(alertLED, OUTPUT);
//  pinMode(alertSpeaker, OUTPUT);
//  pinMode(dndLED, OUTPUT);

 // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  
  // Start the radio listening for data
  radio.startListening();
}

void loop() {
/****************** Pong Back Role ***************************/

  if ( role == 0 )
  {
    unsigned long got_time;
    char text[32] = {0};
    
    if( radio.available()){
                                                                    // Variable for the received timestamp
      while (radio.available()) {                                   // While there is data ready
        radio.read( &got_time, sizeof(unsigned long) );
        radio.read( &text, sizeof(char) );             // Get the payload
        if (got_time == 26984){
          Serial.println("Out of range!!!");
          digitalWrite(alertLED, HIGH);
        }
      }
      
      digitalWrite(alertLED, LOW);
      radio.stopListening();                                        // First, stop listening so we can talk   
      radio.write(&text, sizeof(char));              // Send the final one back.      
      radio.startListening();                                       // Now, resume listening so we catch the next packets.     
      Serial.print(F("Sent response"));
      Serial.println(got_time);  
   }
 }
} // Loop

