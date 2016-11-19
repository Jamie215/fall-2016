#include <QueueList.h>
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

#include <SPI.h>
#include "RF24.h"

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(9,53);
/**********************************************************/

QueueList <unsigned long> runVal;
unsigned long runAvg = 0;
unsigned long runSum = 0;
const unsigned long DELAY_CONST = 500;
const unsigned long THRESHOLD = 1820; // delay-distance threshold (set to 2000);

// buttonPin is for the user to turn off the alert 
const int buttonPin = 13;

byte addresses[][6] = {"1Node","2Node"};

// Used to control whether this node is sending or receiving
bool role = 0;
int RUN_LENGTH = 5 ;
int numFail = 0;
//unsigned long loopCounter = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  radio.begin();
  const int garbage = 0;
  pinMode(buttonPin, INPUT);
  for(int k = 0; k < RUN_LENGTH; k++){
    runVal.push(garbage);
  }

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
  
  
/****************** Ping Out Role ***************************/  
if (role == 1)  {
    
    radio.stopListening();                                    // First, stop listening so we can talk.
    
    
    Serial.println(F("Now sending"));

    unsigned long start_time = micros();                             // Take the time, and send it.  This will block until complete
     if (!radio.write( &start_time, sizeof(unsigned long) )){
       Serial.println(F("failed"));
     }
        
    radio.startListening();                                    // Now, continue listening
    
    unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
    boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
    
    while ( ! radio.available() ){                             // While nothing is received
      if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
          timeout = true;
          break;
      }      
    }
        
    if ( timeout ){                                             // Describe the results
        Serial.println(F("Failed, response timed out."));
        numFail += 1;
//        Serial.print("number of fails: ");
//        Serial.println(numFail);
    }else{
        //loopCounter += 1;
        unsigned long got_time;                                 // Grab the response, compare, and send to debugging spew
        radio.read( &got_time, sizeof(unsigned long) );
        unsigned long end_time = micros();

        runSum -= runVal.pop();
        runVal.push(end_time - start_time);
        runSum += (end_time - start_time);

        runAvg = runSum / RUN_LENGTH;

        if (runAvg > THRESHOLD){
          alert_user();
        }
        
        // Spew it
//        Serial.print(F("Sent "));
        Serial.print(start_time); Serial.println("");
//        Serial.print(F(", Got response "));
//        Serial.print(got_time);
        Serial.print(F(", Round-trip delay "));
        Serial.print(end_time-start_time);
        Serial.println("");
//        Serial.println(F(" microseconds"));
        Serial.print("Running average: ");
        Serial.println(runAvg);
        Serial.print("Running sum: "); Serial.println(runSum);
    }
        
    // Try again 1s later
    delay(DELAY_CONST);
  }



/****************** Pong Back Role ***************************/

  if ( role == 0 )
  {
    unsigned long got_time;
    
    if( radio.available()){
                                                                    // Variable for the received timestamp
      while (radio.available()) {                                   // While there is data ready
        radio.read( &got_time, sizeof(unsigned long) );             // Get the payload
      }
     
      radio.stopListening();                                        // First, stop listening so we can talk   
      radio.write( &got_time, sizeof(unsigned long) );              // Send the final one back.      
      radio.startListening();                                       // Now, resume listening so we catch the next packets.     
      Serial.print(F("Sent response "));
      Serial.println(got_time);  
   }
 }




/****************** Change Roles via Serial Commands ***************************/

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ){      
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = 1;                  // Become the primary transmitter (ping out)
    
   }else
    if ( c == 'R' && role == 1 ){
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));      
       role = 0;                // Become the primary receiver (pong back)
       radio.startListening();
       
    }
  }


} // Loop

void alert_user(){
  // check alerting button state
  bool buttonState = digitalRead(buttonPin);
  
  while(buttonState != HIGH){
    Serial.println("Cane Not Near!");
    buttonState = digitalRead(buttonPin);
    delay(100);
  }
  const int garbage = 0;
  for(int k = 0; k < RUN_LENGTH; k++){
    runVal.pop();
    runVal.push(garbage);
  }
    runSum = 0;
    runAvg = 0;
  
}

