#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

#define HEXPRINT(width, val) setw(width) << setfill('0') << hex << (int(val))

typedef unsigned char byte;

char * c_str(char *p, size_t len);

void dump_hex(const byte * const bytes, size_t size, const char * const msg);

#endif /* UTILS_H */
