/**
 BufferStream.h
 Can read/write at most 1600 chars
 which is sufficient for pfodParser and max MTU
*/

#ifndef BufferStream_H_
#define BufferStream_H_

#include "SafeString.h"

class BufferStream: public Stream  {
public:
    BufferStream();
    ~BufferStream();
    size_t write(const uint8_t *buffer, size_t size) override;
    size_t write(uint8_t data) override;

    int available() override;
    int availableForWrite() override;
    int read() override;
    int peek() override;
    void flush() override;
    void clear();
    const char* getBuffer(); // null terminated
protected:
    char buffer[1400+1]; // add 1 for trailing '\0'
    SafeString* sfPtr; // = SafeString(maxBufferSize, writeBuffer, NULL);

};


#endif /* BufferStream_H_ */
