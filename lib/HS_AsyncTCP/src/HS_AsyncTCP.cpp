#include <Arduino.h>
#include "HS_AsyncTCP_base.h"
#include "BufferStream.h"
#include "StreamBuffers.h"
#include "millisDelay.h"  // from SafeString library

// user code must implement these methods
extern void asyncSetup();
extern void asyncConnected(Stream &stream);
extern void asyncDisconnected();
extern void asyncDataReceived(Stream &stream);
extern void asyncLoop(Stream &stream);

static millisDelay connectionTimeout;
static unsigned long connection_timeout_ms = 0;
static void connect_cb(void* arg, HS_AsyncClient* newClientPtr);
static Stream *wifiDebugPtr;

static HS_AsyncServer *serverPtr;
static HS_AsyncClient* clientPtr; // set to NULL on disconnect
static volatile bool disconnectNow = false;

static BufferStream inBuffer;
static BufferStream outBuffer;

static StreamBuffers streamBuffers(inBuffer, outBuffer);

// call this to set connection timeout if sending keepalives
// else leave as 0 i.e. not connection timeout
void asyncConnectionTimeout(unsigned long ms_timeout) {
  connection_timeout_ms = ms_timeout;
}

void setAsyncDebugPtr(Stream *debugPtr) {
  wifiDebugPtr = debugPtr;
}

bool initAsyncServer(uint16_t port) {
  if (serverPtr) {
    return true; // already done
  }
  serverPtr = new HS_AsyncServer(port);
  if (!serverPtr) {
    return false;
  }
  serverPtr->setNoDelay(true);
  serverPtr->onClient(connect_cb, 0);
  return serverPtr->begin();
}

void asyncCloseConnection() {
  disconnectNow = true;
}

static void stopClient() {
  streamBuffers.clear(); // clean up
  if (clientPtr) {
    clientPtr->stop();
    clientPtr = NULL;
  }
  disconnectNow = false;
}

static void loopHandler(void* arg, HS_AsyncClient* newClientPtr) {
  if (connectionTimeout.justFinished() || disconnectNow) {
    stopClient();
    return;
  }

  asyncLoop(streamBuffers);
  if (wifiDebugPtr) {
    if (outBuffer.available()) {
      wifiDebugPtr->println(outBuffer.getBuffer());
    }
  }
  if (clientPtr && (clientPtr->connected()) && (clientPtr->canSend())) {
    if (outBuffer.available()) {
      clientPtr->write(outBuffer.getBuffer(), outBuffer.available());
    }
  } else {
    if (wifiDebugPtr) {
      wifiDebugPtr->println("skip write as not connected or waiting for Ack");
    }
  }
  outBuffer.clear();
}

// not used
static void pollHandler(void* arg, HS_AsyncClient* newClientPtr) {
  if (wifiDebugPtr) {
    wifiDebugPtr->print("got poll"); wifiDebugPtr->println();
  }
}

static void ackHandler(void* arg, HS_AsyncClient* clientPtr, size_t len, uint32_t time) {
  if (wifiDebugPtr) {
    wifiDebugPtr->print("got ack len:"); wifiDebugPtr->print(len); wifiDebugPtr->print(" time:"); wifiDebugPtr->println(time);
  }
  if (connection_timeout_ms) {
    connectionTimeout.restart();
  }
}

static void dataRecieved_cb(void*arg, HS_AsyncClient*client, void *data, size_t len) {
  if (wifiDebugPtr) {
    wifiDebugPtr->print("dataRecieved_cb core:"); wifiDebugPtr->println(xPortGetCoreID());
  }
  if (connection_timeout_ms) {
    connectionTimeout.restart();
  }
  inBuffer.clear();
  inBuffer.write((uint8_t*)data, len);
  if (wifiDebugPtr) {
    wifiDebugPtr->println("got data");
    wifiDebugPtr->println(inBuffer.getBuffer());
  }
  asyncDataReceived(streamBuffers); // may write response msg
  if (wifiDebugPtr) {
    wifiDebugPtr->println(outBuffer.getBuffer());
  }
  // write response if any
  if (clientPtr && (clientPtr->connected())) {
    if (outBuffer.available()) {
      clientPtr->write(outBuffer.getBuffer(), outBuffer.available());
    }
  } else {
    if (wifiDebugPtr) {
      wifiDebugPtr->println("skip write as not connected");
    }
  }
  outBuffer.clear();
}

static void disconnect_cb(void* arg, HS_AsyncClient* existingClientPtr) {
  // have disconnected so clean up this client
  if (wifiDebugPtr) {
    wifiDebugPtr->println("disconnected");
  }
  connectionTimeout.stop();
  stopClient(); // cleans up
  delete existingClientPtr;
  asyncDisconnected(); // inform user
}

static void connect_cb(void* arg, HS_AsyncClient* newClientPtr) {
  if (wifiDebugPtr) {
    wifiDebugPtr->println("got server connect");
  }
  if (connection_timeout_ms) {
    connectionTimeout.start(connection_timeout_ms);
  }
  clientPtr = newClientPtr;
  clientPtr->onData(dataRecieved_cb);
  clientPtr->onDisconnect(disconnect_cb);
  clientPtr->onAck(ackHandler);
  //  clientPtr->onPoll(pollHandler); // not used
  clientPtr->onLoop(loopHandler);
  streamBuffers.clear();
  asyncConnected(streamBuffers);
}
