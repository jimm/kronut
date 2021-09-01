#include <iostream>
#include <iomanip>
#include <string.h>
#include "utils.h"

using namespace std;

static char c_str_buf[1024];

// Copies "internal representation" p to the pre-allocated buffer c_str_buf
// and returns a pointer to that buffer. Obviously not thread safe.
//
// If p is 0, len is ignored and the pointer to c_str_buf is returned
// without modifying it. This is a way to get the most recently convered
// string without re-converting it.
char * c_str(char *p, size_t len) {
  if (p != 0) {
    memcpy(c_str_buf, p, len);
    c_str_buf[len] = 0;
  }
  return c_str_buf;
}

void dump_hex(byte *bytes, size_t size, const char * const msg) {
  cout << msg << endl;
  if (bytes == 0) {
    cout << "<null>" << endl;
    return;
  }
  if (size == 0) {
    cout << "<empty>" << endl;
    return;
  }
  size_t offset = 0;
  while (size > 0) {
    int chunk_len = 8 > size ? size : 8;
    cout << setw(8) << setfill('0') << hex << offset << ' ';
    cout << "  ";
    for (int i = 0; i < chunk_len; ++i)
      cout << ' ' << setw(2) << setfill('0') << hex << (int)bytes[i];
    for (int i = chunk_len; i < 8; ++i)
      cout << "   ";
    cout << ' ';
    for (int i = 0; i < chunk_len; ++i)
      cout << (char)((bytes[i] >= 32 && bytes[i] < 127) ? bytes[i] : '.');
    cout << endl;
    bytes += chunk_len;
    size -= chunk_len;
    offset += chunk_len;
  }
}
