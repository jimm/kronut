#pragma once

#include <string>
#include "types.h"

using namespace std;

// Provides methods for handling data within data structs. Currently handles
// Kronos strings which can be up to X bytes long and are not necessarily
// null-terminated. For example is, a Kronos string of length 24 may use all
// 24 characters and not have any null byte.
class StructWrapper {
protected:
  // Reads up to len bytes and returns a string. Handles non-null terminated
  // byte buffers.
  string chars_to_string(byte *, int len);

  // Writes a string into a byte array, handling truncation and null
  // termination. Returns 0 if `str` is longer than `len`, else returns 0.
  int string_to_chars(byte *, int len, string str);
};
