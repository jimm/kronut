#ifndef MOCK_KRONOS_H
#define MOCK_KRONOS_H

#include <string>
#include "kronos.h"

using namespace std;

typedef unsigned char byte;

class MockKronos : public Kronos {
public:
  string name;
  string comments;

  MockKronos(byte channel);
  ~MockKronos();

  void receive_midi(const MIDIPacketList *pktlist) {}
  void write_current_slot_name(KString *kstr) { name = kstr->str(); }
  void write_current_slot_comments(KString *kstr) { comments = kstr->str(); }

protected:
  void read_sysex() {}
  KString * read_current_string(int obj_type, byte pad);
  void send_sysex(const byte * const sysex, UInt32 bytes_to_send) {}
  void write_current_string(int obj_type, KString *kstr) {}
};

#endif /* MOCK_KRONOS_H */
