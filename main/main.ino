#include <SPI.h>
#include <Ethernet.h>
#include "globals.h"
#include "rpcserver.h"
#include "hardware.h"
#include "interrupt.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,177);
//IPAddress ip(169,254,1,1);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);


void setup() {
  memset(rxLine,0,sizeof(rxLine));
  rxLinePos = 0;

  hardwareInit();
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac,ip);
  server.begin();
  
  // Setup interrupts
  interruptInit();  
}

ISR(TIMER1_COMPA_vect) { 
  // Every REFRESH_SECONDS seconds we'll refresh the local cache
  interruptHandleTimer();
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();        

        // Buffer lines as they arrive
        if (rxLinePos < sizeof(rxLine) - 2) {
          rxLine[rxLinePos++] = c;
        }

        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
          parseBuffer();
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          busy = 1;
          switch(cmd) {
            case CMD_STATUS:
              hdlStatus(client);
              break;
            case CMD_OPEN:
              hdlOpen(client);
              break;
            case CMD_CLOSE:
              hdlClose(client);
              break;
            case CMD_RESET:
              hdlReset(client);
              break;
            default: 
              sendResponse(client, STATUS_NOT_FOUND);
              break;
          }
          cmd = CMD_NONE;
          busy = 0;

          break;
        }
      }
    }
    // give the web browser time to receive the data - ToDo: is this necessary?
    delay(1);
    
    // close the connection:
    client.stop();
  }
}


