/**
  BufferStream.cpp

*/

#include <Arduino.h>
#include "BufferStream.h"

BufferStream::BufferStream() {
  sfPtr = new SafeString(sizeof(buffer), buffer, NULL);
}

BufferStream::~BufferStream() {
  delete sfPtr;
}

void BufferStream::clear() {
  sfPtr->clear();
}

const char* BufferStream::getBuffer() {
  return sfPtr->c_str(); // null terminated
}

int BufferStream::availableForWrite() {
  return (sfPtr->availableForWrite());
}

size_t BufferStream::write(const uint8_t *data, size_t size) {
  if (data && size) {
    sfPtr->readFrom((char*)data, size);
    return size;
  }
  return 0;
}

size_t BufferStream::write(uint8_t data) {
  sfPtr->concat((char)data);
  return 1;
}

int BufferStream::available() {
  return sfPtr->length();
}

int BufferStream::read() {
  if (sfPtr->length()) {
    char c = sfPtr->charAt(0);
    sfPtr->remove(0, 1);
    return c;

  }
  return -1;
}

int BufferStream::peek() {
  if (sfPtr->length()) {
    char c = sfPtr->charAt(0);
    return c;
  }
  return -1;
}

void BufferStream::flush() {
}
