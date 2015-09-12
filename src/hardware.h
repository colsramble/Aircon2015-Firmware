#ifndef AC_HARDWARE_H
#define AC_HARDWARE_H

// Pins
#define PIN_ZONE_TEMP   0
#define PIN_100_DETECT  8

#define PIN_ANALOG_MIN  1
#define PIN_ANALOG_MAX  5

// Constants
#define ZONE_OPEN          0
#define ZONE_CLOSE         1

// Shortcuts
#define LED_ON          PORTD |= B10000000
#define LED_OFF         PORTD &= B01111111

#define ENERGISE_ON     PORTD |= B01000000
#define ENERGISE_OFF    PORTD &= B10111111

#define CLOSE_ON        PORTD |= B00100000
#define CLOSE_OFF       PORTD &= B11011111

// Prototypes
void hardwareInit();
void calibrate();
void runZone(int, int, int);
int readZoneTemp(int);
void selectZone(int);

#endif
