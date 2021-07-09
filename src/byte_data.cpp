#include <string.h>
#include "byte_data.h"

#define CHUNK_SIZE 1024

ByteData::ByteData() {
  bytes = (byte *)malloc(CHUNK_SIZE);
  allocated_size = CHUNK_SIZE;
  length = 0;
}

ByteData::~ByteData() {
  free(bytes);
}

void ByteData::clear() {
  length = 0;
}

void ByteData::append(byte b) {
  if (length == allocated_size) {
    allocated_size += allocated_size;
    bytes = (byte *)realloc(bytes, allocated_size);
  }
  bytes[length++] = b;
}

void ByteData::append(byte *b, int len) {
  if (length + len >= allocated_size) {
    allocated_size += allocated_size;
    bytes = (byte *)realloc(bytes, allocated_size);
  }
  memcpy(bytes + length, b, len);
  length += len;
}
