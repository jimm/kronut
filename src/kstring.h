#ifndef KSTRING_H
#define KSTRING_H

#include "midi_data.h"

class KString : public MIDIData {
public:
  KString(int init_type, byte *bytes, size_t len, byte pad = 0);
  KString(int init_type, char *chars, size_t len, byte pad = 0);
  ~KString();

  char * str() { return c_str; }
  void set_str(char *str) { set_str((const char * const)str); }
  void set_str(const char * const str);

  virtual void midi_changed();
  virtual void internal_changed();

  void str_changed() { internal_changed(); }

  void dump();

private:
  byte pad;
  char *c_str;

  void internal_bytes_to_str();
};

#endif /* KSTRING_H */