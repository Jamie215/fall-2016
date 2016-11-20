
/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"
#include <QueueList.h>
/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
/**********************************************************/

byte addresses[][6] = {"1Node","2Node"};

// Used to control whether this node is sending or receiving
bool role = 0;

QueueList <unsigned long> runVal;
unsigned long runAvg = 0;
unsigned long runSum = 0;
const unsigned long DELAY_CONST = 100;
const unsigned long THRESHOLD = 2000; // delay-distance threshold (set to 2000);

// buttonPin is for the user to turn off the alert 
const int buttonPin = 4;
const int garbage = 0;
const int alertLED = 9;
int RUN_LENGTH = 150;
int numFail = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));
  
  radio.begin();
  pinMode(alertLED, OUTPUT);
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
  }
  else{
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
    }
    else{
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
//        Serial.print("Running average: ");
        Serial.println(runAvg);
//        Serial.print("Running sum: "); Serial.println(runSum);
    }

    // Try again 1s later
    delay(DELAY_CONST);
    
  }

} // Loop
void alert_user(){
  // check alerting button state
  bool buttonState = digitalRead(buttonPin);
  radio.stopListening();
  char text[32] = "hi";
  radio.write(&text, sizeof(text));
  
  while(buttonState != LOW){
    Serial.println("Don't forget your walker!");
    radio.write(&text, sizeof(text));
    buttonState = digitalRead(buttonPin);
    digitalWrite(alertLED,HIGH);
    delay(100);
    digitalWrite(alertLED,LOW);
  }
  char hi[32] = "hello";
  radio.write(&text, sizeof(text));
  const int garbage = 0;
  for(int k = 0; k < RUN_LENGTH; k++){
    runVal.pop();
    runVal.push(garbage);
  }
    runSum = 0;
    runAvg = 0;
  
}
