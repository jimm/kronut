#ifndef UTILS_H
#define UTILS_H

typedef unsigned char byte;

char * c_str(char *p, size_t len);

void dump_hex(byte *bytes, size_t size, const char * const msg);

#endif /* UTILS_H */