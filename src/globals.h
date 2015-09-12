#ifndef AC_GLOBALS_H
#define AC_GLOBALS_H

// Hardware configuration   
#define ZONE_MIN           1
#define ZONE_MAX           5
#define ZONE_MAX_RUN_TIME  20000  // milliseconds
  
// Request Params
extern int  param1;
extern int  param2;

// Response params
extern int respStatus;  // 200, 404
extern int respParam1;
extern int respParam2;

// Zone data tables
// (only using zones 1 to 5)
extern int zonePosition[];     // = { 0, 0, 0, 0, 0, 0 };
extern int zoneCalibration[];  // = { 0, ZONE_MAX_RUN_TIME, ZONE_MAX_RUN_TIME, ZONE_MAX_RUN_TIME, ZONE_MAX_RUN_TIME, ZONE_MAX_RUN_TIME } ;
extern int zoneTemp[];         // = { 0, 0, 0, 0, 0, 0 };

#endif
