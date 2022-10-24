#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

typedef unsigned char byte;

char * c_str(char *p, size_t len);

void dump_hex(const byte * const bytes, size_t size, const char * const msg);

#endif /* UTILS_H */
