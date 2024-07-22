/**
  StreamBuffers.cpp
*/

#include <Arduino.h>
#include "StreamBuffers.h"

StreamBuffers::StreamBuffers(BufferStream &in, BufferStream &out) {
  inPtr = &in;
  outPtr = &out;
}

void StreamBuffers::clear() {
  inPtr->clear();
  outPtr->clear();
}

size_t StreamBuffers::write(const uint8_t *data, size_t size) {
  return outPtr->write(data,size);
}

size_t StreamBuffers::write(uint8_t data) {
  return outPtr->write(data);
}

int StreamBuffers::availableForWrite() {
  return (outPtr->availableForWrite());
}

int StreamBuffers::available() {
  return inPtr->available();
}

int StreamBuffers::read() {
  return inPtr->read();
}

int StreamBuffers::peek() {
  return inPtr->peek();
}

void StreamBuffers::flush() {
}
