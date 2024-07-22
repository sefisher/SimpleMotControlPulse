#ifndef HS_ASYNCTCP_H_
#define HS_ASYNCTCP_H_

#include <Stream.h> 

bool initAsyncServer(uint16_t port);

void asyncCloseConnection();
void asyncConnectionTimeout(unsigned long msTimeout); // default on startup is 0, never timeout

// ============= Start of methods that need to be implemented by user =====================

void setAsyncDebugPtr(Stream *debugPtr);
/**
 * Your code MUST implement these 3 methods
  void asyncConnected(Stream &stream); 
  void asyncDisconnected(); 
  void asyncDataReceived(Stream &stream); 
  void asyncLoop(Stream &stream); 
 */

/**
 * asyncConnected
 * runs on WiFi core 0, in HS_Async thread (task). Use volatiles to interact with your Arduino loop() code
 * Called on new connection
 * Text written to stream sent on return from this method.
 */
void asyncConnected(Stream &stream);  

/**
 * asyncDisconnected
 * runs on WiFi core 0, in HS_Async thread (task). Use volatiles to interact with your Arduino loop() code
 * Called when connection closed, either by other side or by your code calling asyncCloseConnection();
 * Text written to stream sent on return from this method.
 */
void asyncDisconnected(); 

/**
 * asyncDataReceived
 * runs on WiFi core 0, in HS_Async thread (task). Use volatiles to interact with your Arduino loop() code
 * Called on when data received from WiFi connection.
 * All incoming data should be processed in this method. Any unprocessed data will be discared on return from this method
 * Text written to stream sent on return from this method.
 */
void asyncDataReceived(Stream &stream); 

/**
 * asyncLoop
 * runs on WiFi core 0, in HS_Async thread (task). Use volatiles to interact with your Arduino loop() code
 * Called on average every 1.5ms but can be upto 20ms between calls if the WiFi is busy
 * Text written to stream sent on return from this method.
 */
void asyncLoop(Stream &stream); 

// ============= end of methods that need to be implemented by user =====================


#endif
