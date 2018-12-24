#include <string.h>
#include "kstring.h"
#include "utils.h"

KString::KString(int init_type, byte *bytes, size_t len, byte p)
  : MIDIData(init_type, bytes, len), c_str(0), pad(p)
{
  internal_bytes_to_str();
}

KString::~KString()
{
  if (c_str)
    free(c_str);
}

void KString::set_str(const char * const str) {
  if (c_str)
    free(c_str);
  c_str = (char *)malloc(internal_len + 1);
  memcpy(c_str, str, internal_len);
  c_str[internal_len] = 0;

  bool padding = false;
  for (int i = 0; i < internal_len; ++i) {
    if (c_str[i] == 0) {
      memset(&internal_bytes[i], pad, internal_len - i);
      break;
    }
    if (c_str[i] == '\n')
      internal_bytes[i] = '\r';
    else
      internal_bytes[i] = c_str[i];
  }
  MIDIData::internal_changed();
}

void KString::midi_changed() {
  MIDIData::midi_changed();
  internal_bytes_to_str();
}

void KString::internal_changed() {
  MIDIData::internal_changed();
  internal_bytes_to_str();
}

void KString::dump() {
  MIDIData::dump();
  dump_hex((byte *)c_str, strlen(c_str) + 1, "KString c_str");
}

// Copy internal bytes to str, replacing padding byte with 0 and replacing
// \r with \n. (Kronos strings are stored with \r newlines, not DOS \r\n or
// Unix \n.)
void KString::internal_bytes_to_str() {
  if (c_str)
    free(c_str);
  c_str = (char *)malloc(internal_len + 1);
  memcpy(c_str, internal_bytes, internal_len);
  c_str[internal_len] = 0;

  char *p = c_str + internal_len - 1;
  // Turn trailing pad chars, spaces, and carriage returns into 0s.
  while (p >= c_str && (*p == pad || *p == '\r' || *p == ' '))
    *p-- = 0;
  for (char *r = c_str; r < p; ++r)
    if (*r == '\r')
      *r = '\n';
}
