#ifndef StreamBuffers_H_
#define StreamBuffers_H_

#include "BufferStream.h"

class StreamBuffers: public Stream  {
public:
    StreamBuffers(BufferStream &in, BufferStream &out);

    size_t write(const uint8_t *buffer, size_t size) override;
    size_t write(uint8_t data) override;

    int available() override;
    int availableForWrite() override;
    int read() override;
    int peek() override;
    void flush() override;
    void clear();
protected:
    BufferStream *inPtr;
    BufferStream *outPtr;
};

#endif
