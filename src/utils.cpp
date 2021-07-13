#include <stdio.h>
#include <string.h>
#include "utils.h"

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
  printf("%s\n", msg);
  if (bytes == 0) {
    puts("<null>");
    return;
  }
  if (size == 0) {
    puts("<empty>");
    return;
  }
  size_t offset = 0;
  while (size > 0) {
    int chunk_len = 8 > size ? size : 8;
    printf("%08lx:", offset);
    for (int i = 0; i < chunk_len; ++i) {
      printf(" %02x", bytes[i]);
    }
    for (int i = chunk_len; i < 8; ++i) {
      printf("   ");
    }
    printf(" ");
    for (int i = 0; i < chunk_len; ++i) {
      printf("%c", (bytes[i] >= 32 && bytes[i] < 127) ? bytes[i] : '.');
    }
    puts("");
    bytes += chunk_len;
    size -= chunk_len;
    offset += chunk_len;
  }
}
