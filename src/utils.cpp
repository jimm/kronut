#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h>
#include "utils.h"

using namespace std;

vector<string> string_to_words(const char * const str) {
    stringstream sstr(str);
    string word;
    vector<string> words;

    while (sstr >> word)
        words.push_back(word);
    return words;
}

void dump_hex(const byte * const bytes, size_t size, const char * const msg) {
  const byte *p = (const byte *)bytes;

  cout << msg << "\n";
  if (p == 0) {
    cout << "<null>\n";
    return;
  }
  if (size == 0) {
    cout << "<empty>\n";
    return;
  }
  size_t offset = 0;
  while (size > 0) {
    int chunk_len = 8 > size ? size : 8;
    cout << HEXPRINT(8, offset) << ' ';
    cout << "  ";
    for (int i = 0; i < chunk_len; ++i)
      cout << ' ' << HEXPRINT(2, p[i]);
    for (int i = chunk_len; i < 8; ++i)
      cout << "   ";
    cout << ' ';
    for (int i = 0; i < chunk_len; ++i)
      cout << (char)((p[i] >= 32 && p[i] < 127) ? p[i] : '.');
    cout << "\n";
    p += chunk_len;
    size -= chunk_len;
    offset += chunk_len;
  }
}
