#ifndef MOCK_KRONOS_H
#define MOCK_KRONOS_H

#include <string>
#include "../src/kronos.h"

using namespace std;

typedef unsigned char byte;

class MockKronos : public Kronos {
public:
  string name;
  string comments;

  MockKronos(byte channel);
  ~MockKronos();

  void receive_midi() {}
  void write_current_slot_name(KString *kstr) { name = kstr->str(); }
  void write_current_slot_comments(KString *kstr) { comments = kstr->str(); }

protected:
  void send_sysex(const byte * const sysex) {}
  void read_sysex() {}
  void get(const byte * const, const char * const) {}
  void send_channel_message(byte, byte, byte) {}

  KString * read_current_string(int obj_type, byte pad);
  void write_current_string(int obj_type, KString *kstr) {}
};

#endif /* MOCK_KRONOS_H */
