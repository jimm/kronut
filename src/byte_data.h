#ifndef BYTE_DATA_H
#define BYTE_DATA_H

#include <stdlib.h>

typedef unsigned char byte;

class ByteData {
public:
  ByteData();
  ~ByteData();

  inline byte * data() { return bytes; }
  inline size_t size() { return length; }

  inline byte operator[](int i) { return bytes[i]; }

  void clear();
  void append(byte b);

private:
  byte *bytes;
  size_t allocated_size;
  size_t length;
};

#endif /* BYTE_DATA_H */