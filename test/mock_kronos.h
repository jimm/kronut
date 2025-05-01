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

  MockKronos(byte channel);     // channel 1-15
  virtual ~MockKronos();

  void receive_midi() {}
  void write_current_slot_name(KString *kstr) override { name = kstr->str(); }
  void write_current_slot_comments(KString *kstr) override { comments = kstr->str(); }

protected:
  bool send_sysex(const char * const func_name, vector<byte> &sysex) override
    { return true; }
  bool read_sysex(const char * const func_name, byte reply_function) override
    { return true; }
  bool get(vector<byte> &request_sysex, const char * const func_name, byte reply_function) override
    { return true; }
  void send_channel_message(byte, byte, byte) override {}

  KString * read_current_string(int obj_type, byte pad) override;
  void write_current_string(int obj_type, KString *kstr) override {}
};

#endif /* MOCK_KRONOS_H */
