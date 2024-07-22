#ifndef WIFI_DATA_HANDLING_H_
#define WIFI_DATA_HANDLING_H_
/*
   (c)2023 Forward Computing and Control Pty. Ltd.
   NSW Australia, www.forward.com.au
   This code is not warranted to be fit for any purpose. You may only use it at your own risk.
   This generated code may be freely used for both private and commercial use
   provided this copyright is maintained.
*/

#include <Arduino.h>

/**
   Your code MUST implement these 5 methods, can be just empty methods
  void asyncSetup();
  void asyncLoop(Stream &stream);
  void asyncConnected(Stream &stream);
  void asyncDisconnected();
  void asyncDataReceived(Stream &stream);
*/

/**
   asyncSetup
   runs on WiFi core 0, in HS_Async thread (task). Use volatiles to interact with your Arduino loop() code
   Called just once when the HS_AsyncTCP task on core 0 starts
   Use this to do any initialization of WiFi task local variables, e.g. start timers etc
*/
void asyncSetup();

/**
   asyncLoop
   runs on WiFi core 0, in HS_Async thread (task). Use volatiles to interact with your Arduino loop() code
   Called on average every 1.5ms but can be upto 20ms between calls if the WiFi is busy
   Text written to stream sent on return from this method.
*/
void asyncLoop(Stream &stream);

/**
   asyncConnected
   runs on WiFi core 0, in HS_Async thread (task). Use volatiles to interact with your Arduino loop() code
   Called on new connection
   Text written to stream sent on return from this method.
*/
void asyncConnected(Stream &stream);

/**
   asyncDisconnected
   runs on WiFi core 0, in HS_Async thread (task). Use volatiles to interact with your Arduino loop() code
   Called when connection closed, either by other side or by your code calling asyncCloseConnection();
   Text written to stream sent on return from this method.
*/
void asyncDisconnected();

/**
   asyncDataReceived
   runs on WiFi core 0, in HS_Async thread (task). Use volatiles to interact with your Arduino loop() code
   Called on when data received from WiFi connection.
   All incoming data should be processed in this method. Any unprocessed data will be discared on return from this method
   Text written to stream sent on return from this method.
*/
void asyncDataReceived(Stream &stream);

// ============= end of methods that need to be implemented =====================

/**
   asyncCloseConnection()
   flags connection to be closed
   Can be safely called from any core or thread
*/
void asyncCloseConnection();

/**
   asyncConnectionTimeout()
   sets the connection timeout. Connection is closed if no data sent or received for this period of time
   
   WiFi and Ethernet connections require special handling because the connection can end being 'half closed',
   which happen went the client just disappears due to a bad WiFi connection, power loss at the router or forced shut down of the client.
   See Detection of Half-Open (Dropped) TCP/IP Socket Connections (http://www.codeproject.com/Articles/37490/Detection-of-Half-Open-Dropped-TCP-IP-Socket-Conne)
   or this copy (https://www.pfod.com.au/pfod/pfodLinkDesign/Detection%20of%20Half-Open%20(Dropped)%20TCP_IP%20Socket%20Connections%20-%20CodeProject.pdf)
   for more details.

   Leave as 0 if you don't regualarly send data or receive commands.
   Can be safely called from any core or thread
*/
void asyncConnectionTimeout(unsigned long msTimeout); // default on startup is 0, never timeout



#endif
