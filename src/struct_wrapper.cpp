#include "struct_wrapper.h"

// Reads up to len bytes and returns a string. Handles non-null terminated
// byte buffers.
string StructWrapper::chars_to_string(byte *chars, int len) {
  char buf[BUFSIZ];

  memcpy(buf, chars, len);
  buf[len] = '\0';
  return string(buf);
}

// Writes a string into a byte array, handling truncation and null
// termination. Returns 0 if `str` is longer than `len`, else returns 0.
int StructWrapper::string_to_chars(byte *chars, int len, string str) {
  memcpy(chars, str.c_str(), len);
  if (str.size() < len)
    memset(chars + str.size(), 0, len - str.size());

  return str.size() > len;
}
