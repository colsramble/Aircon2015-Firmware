#include "Arduino.h"
#include "globals.h"
#include "hardware.h"

// Status tables
int zonePosition[]    = { 0, 0, 0, 0, 0, 0 };
int zoneCalibration[] = { 0, ZONE_MAX_RUN_TIME, ZONE_MAX_RUN_TIME, ZONE_MAX_RUN_TIME, ZONE_MAX_RUN_TIME, ZONE_MAX_RUN_TIME } ;
int zoneTemp[]        = { 0, 0, 0, 0, 0, 0 };
int analogChannels[]  = { 0, 0, 0, 0, 0, 0 };

// Refresh settings and global "busy" flag
int refreshCounter = REFRESH_SECONDS;  
int busy = 0;

void calibrate() {
  /* 
     foreach zone 
       - select zone
       - close it fully
       - start timer
       - open fully
       - stop timer
     
     select zone 0
  */
  for(int z=ZONE_MIN; z<=ZONE_MAX; z++) {
    // get zone temp
    readZoneTemp(z);
    
    runZone(z, 100, ZONE_CLOSE);   // Close 100%
    delay(1000);
    
    runZone(z, 100, ZONE_OPEN);    // Open 100% (gets full articulation time)
    // Update calibration table
    zoneCalibration[z] = zonePosition[z];    
  }
}

void runZone(int z, int percent, int mode) {
  // Will energize given zone according to mode for a length of time 
  // determined by percentage and callibration table or until fully articulated
  // After execution zonePosition will be updated to reflect new position
  if ( z < ZONE_MIN || z > ZONE_MAX ) {
    return;
  }
  if (percent <= 0) {
    // nothing to do
    return;
  }
  if (percent > 100) {
    percent = 100;
  }

  // convert percent to run time
  //float runTime = (float) percent / 100.0;
  //runTime = runTime * (float) zoneCalibration[z];    // float run time in milliseconds
  //int iRunTime = (int) runTime;                      // integer run time in milliseconds
  
  unsigned long runTime = (unsigned long) percent * (unsigned long) zoneCalibration[z];
  runTime = runTime / (unsigned long) 100;
  int iRunTime = (int) runTime;   // integer run time in milliseconds

  byte isMidPosition = 1;
  
  // 100% ? then override to run to max
  if (percent == 100) {
    iRunTime = ZONE_MAX_RUN_TIME;
  }
  
  selectZone(z);

  if (ZONE_CLOSE == mode) {
    CLOSE_ON;
    // Relay response time
    delay(50);
  }
  ENERGISE_ON;
  
  unsigned long start = millis();
  // Run until time is spent or fully open/closed
  while ( iRunTime > 0 && isMidPosition ) {
    delay(10);
    iRunTime = iRunTime - 10;
    isMidPosition = digitalRead(PIN_100_DETECT);
  }
  ENERGISE_OFF;
  unsigned long finish = millis();
  CLOSE_OFF; 

  unsigned long actualRunTime = 0;
  if (finish > start) {
    actualRunTime = finish - start;
  } else {
    actualRunTime = start - finish;
  }
  
  // done with the zone
  selectZone(0);
  
  // Update state
  if (ZONE_CLOSE == mode) {
    zonePosition[z] = zonePosition[z] - (int) actualRunTime;
    if (!isMidPosition || zonePosition[z] < 0) {
      // fully closed
      zonePosition[z] = 0;
    }
  } else {
    if (!isMidPosition) {
      // Full motion
      if (zonePosition[z] == 0) {
        // .. and started from full closed, so calibrate
        zoneCalibration[z] = (int) actualRunTime;
      }
      zonePosition[z] = zoneCalibration[z];
    } else {
      zonePosition[z] = zonePosition[z] + (int) actualRunTime;
    }
  }  
}

int readZoneTemp(int zone) {
  if ( zone < ZONE_MIN || zone > ZONE_MAX ) {
    return 0;
  }
  
  selectZone(zone);
  
  // small settle time
  delay(10);  // settling time?
  
  // average a number of samples
  unsigned int temp = 0;
  for (int i=0; i<64; i++) {
    temp += analogRead(PIN_ZONE_TEMP);
  } 
  temp = temp / 64;
  
  zoneTemp[zone] = (int) temp;
  
  return zoneTemp[zone];
}

void hardwareInit() {
  /*
     set up I/O
     
     PIN   PortRef    Direction    Usage
     D2    D2         OUT          ZoneSelect0
     D3    D3         OUT          ZoneSelect1
     D4    D4         OUT          ZoneSelect2

     D5    D5         OUT          close
     D6    D6         OUT          energise
     D7    D7         OUT          LED

     D8    B0         IN           100% (active low)
     
     A0    C0         IN           ZoneTemp
  */  
  PORTD &= B00000011;    // Select zone zero and turn everything off
  DDRD  |= B11111100;    // Set all interested pins to OUTPUT 
  
  pinMode(8, INPUT);     // 100% detect
  
  analogReference(DEFAULT);
  
  // Initialise configuration tables
  calibrate();
}

void selectZone(int zone) {
  // Use bitwise operations and direct port access to ensure zone selection is 
  // immediate and traverses zone zero to avoid any transient contention
  zone &= B00000111;
  zone = zone << 2;
  
  // select zone zero (no zone) and turn everything else off
  PORTD &= B00000011;
  
  // delay to ensure no transient contention 
  delay(1);
  
  // select chosen zone
  PORTD |= (byte) zone;

  if (zone > 0) {
    LED_ON;
  } else {
    LED_OFF;  
  }
  // delay to settling time? TODO: Probably not needed! 
  delay(1);
}


