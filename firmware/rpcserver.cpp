#include <SPI.h>
#include <Ethernet.h>
#include "globals.h"
#include "rpcserver.h"
#include "hardware.h"

// Request Params
int  param1;
int  param2;

// Response params
int respStatus;  // 200, 404
int respParam1;
int respParam2;

// Raw received request buffer 
char rxLine[100];
int  rxLinePos;

// Processed request params
byte method;   // "GET" | "POST"
byte cmd;      // "stat" | "open" | "clse" | "rset"

void hdlStatus(EthernetClient client) {
  // param1:  zone to update    
  // param2:  unused
  
  // read zone will select it
  //readZoneTemp(param1);
  
  // Just force a refresh of current status
  refreshStatus();

  sendResponse(client, STATUS_OK);

  // unselect zone
  selectZone(0);
}

void hdlOpen(EthernetClient client) {
  // param1:  zone select   
  // param2:  percentage (0-100)
  
  runZone(param1, param2, ZONE_OPEN);
  
  //int zoneTemp = readZoneTemp();

  // done - send response
  //float posPercent = (float) zonePosition[param1] / (float) zoneCalibration[param1];
  //posPercent = posPercent * 100.0;
  //sendResponse(client, STATUS_OK, param1, zoneTemp, (int) posPercent);
  hdlStatus(client);
}

void hdlClose(EthernetClient client) {
  // param1:  zone select   
  // param2:  percentage (0-100)
  
  runZone(param1, param2, ZONE_CLOSE);
  
  //int zoneTemp = readZoneTemp();

  // done - send response
  //float posPercent = (float) zonePosition[param1] / (float) zoneCalibration[param1];
  //posPercent = posPercent * 100.0;
  //sendResponse(client, STATUS_OK, param1, zoneTemp, (int) posPercent);
  hdlStatus(client);
}

void hdlReset(EthernetClient client) {  
  hardwareInit();
  
  // done - send response
  sendResponse(client, STATUS_OK);
}

void parseParams(char* paramStr) {
  // just grab next three chars and convert to int
  param1 = 0;
  param2 = 0;

  param1  = char2int(paramStr[0]) * 100;
  param1 += char2int(paramStr[1]) * 10;
  param1 += char2int(paramStr[2]);

  if (paramStr[3] == '/') {
    param2  = char2int(paramStr[4]) * 100;
    param2 += char2int(paramStr[5]) * 10;
    param2 += char2int(paramStr[6]);
  }  
}

int char2int( char c ) {
  int response = c - '0';
  
  if (response < 0 || response > 10) {
    response = 0;
  }
  
  return response;
}

void parseBuffer() {
  // Look for command eg/ {GET|POST} {PATH} HTTP/{VERSION}
  // eg {METHOD} = GET  
  //    {PATH}   = "/" + {CMD} + "/" + {param} + [ "/" + {param} ]
  //    {CMD}    = "stat" | "open" | "clse" | "rset"
  //    {param}  = "000" - "255" 
  if (rxLinePos > 2) {

    if (memcmp("GET", rxLine, 3) == 0) {
      method = METHOD_GET;
      cmd    = CMD_NONE;
       
      // determine command     
      if (memcmp("stat", rxLine+5, 4) == 0) {
        cmd = CMD_STATUS;
      } else if (memcmp("open", rxLine+5, 4) == 0) {
        cmd = CMD_OPEN;
      } else if (memcmp("clse", rxLine+5, 4) == 0) {
        cmd = CMD_CLOSE;
      } else if (memcmp("rset", rxLine+5, 4) == 0) {
        cmd = CMD_RESET;
      } 
      
      // if valid command then get params
      if (cmd != CMD_NONE) {
        parseParams( rxLine+10 );
      }
    } 
  }

  // done processing so reset
  memset(rxLine,0,sizeof(rxLine));
  rxLinePos = 0;
}

void sendResponse(EthernetClient client, int status) {
  delay(1000);  // TOD: sometimes short response times fail, so wait a bit 
  
  // Header
  client.print("HTTP/1.1 ");
  client.println(status);
  if (STATUS_OK == status) {
    client.println(" OK");
  } else {
    client.println(" OTHER");
  }
  client.println("Content-Type: application/javascript");
  client.println("Connnection: close");
  client.println();

  // open body
  client.print("aircon2015({");
  
  //status
  client.print("\"status\":");
  client.print(status);
  
  //cmd
  client.print(",");
  client.print("\"cmd\":");
  client.print(cmd);

  //payload
  client.print(",");
  client.print("\"data\": { \"zones\": [ ");
  for (int i=ZONE_MIN; i<=ZONE_MAX; i++) {
    client.print("{\"id\": ");
    client.print(i);
    client.print(", \"temp\": ");
    client.print(zoneTemp[i]);
    client.print(", \"pos\": ");
    client.print(zonePosition[i]);
    client.print(", \"cal\": ");
    client.print(zoneCalibration[i]);
    client.print("}");
    
    if (i<ZONE_MAX) {
      client.print(", ");
    }
  }
  
  client.print(" ], \"analog\": [ ");
  for (int i=PIN_ANALOG_MIN; i<=PIN_ANALOG_MAX; i++) {
    client.print("{\"pin\": ");
    client.print(i);
    client.print(", \"val\": ");
    client.print(analogChannels[i]);
    client.print("}");
  
    if (i<PIN_ANALOG_MAX) {
      client.print(", ");
    }
  }
  client.print(" ] }");
  
  // close body
  client.println("});");  
}

