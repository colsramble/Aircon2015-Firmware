#include "Arduino.h"
#include "globals.h"
#include "hardware.h"

void interruptInit() {
  //stop interrupts
  cli();
  //set timer1 interrupt at 1Hz
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624; // = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); //allow interrupts
}

void interruptHandleTimer() {
 // only do something if not busy and timer has expired 
  if (busy == 0 && refreshCounter-- < 0) { 
    
    // Refresh zone data
    for(int z=ZONE_MIN; z<=ZONE_MAX; z++) {
      // get zone temp
      zoneTemp[z] = readZoneTemp(z);    
    }
    selectZone(0);
    
    // Refresh analog data
    for (int i=PIN_ANALOG_MIN; i<=PIN_ANALOG_MAX; i++) {
      analogChannels[i] = analogRead(i);
    }
    
    refreshCounter = REFRESH_SECONDS;
  }
}
