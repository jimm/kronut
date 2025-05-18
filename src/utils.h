#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <sys/types.h>

#define HEXPRINT(width, val) setw(width) << setfill('0') << hex << (int(val))

typedef unsigned char byte;

std::vector<std::string> string_to_words(const char * const str);
void dump_hex(const byte * const bytes, size_t size, const char * const msg);

#endif /* UTILS_H */
