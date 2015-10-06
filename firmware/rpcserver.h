#ifndef AC_RPCSERVER_H
#define AC_RPCSERVER_H

// METHODS
#define METHOD_GET  1
#define METHOD_POST 2

// COMMANDS
#define CMD_NONE   0
#define CMD_STATUS 1
#define CMD_OPEN   2
#define CMD_CLOSE  3
#define CMD_RESET  4

// Request Params
extern int  param1;
extern int  param2;

// Response params
extern int respStatus;  // 200, 404
extern int respParam1;
extern int respParam2;

// Raw received request buffer 
extern char rxLine[100];
extern int  rxLinePos;

// Processed request params
extern byte method;   // "GET" | "POST"
extern byte cmd;      // "stat" | "open" | "clse" | "rset"

// STATUS
#define STATUS_OK        200
#define STATUS_NOT_FOUND 404

void hdlStatus(EthernetClient);
void hdlOpen(EthernetClient);
void hdlClose(EthernetClient);
void hdlReset(EthernetClient);

void parseBuffer();
void sendResponse(EthernetClient, int);
int char2int(char);

#endif
