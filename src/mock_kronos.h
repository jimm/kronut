#ifndef MOCK_KRONOS_H
#define MOCK_KRONOS_H

#include "kronos.h"

using namespace std;

typedef unsigned char byte;

class MockKronos : public Kronos {
public:
  MockKronos(byte channel);
  ~MockKronos();

  void receive_midi(const MIDIPacketList *pktlist);

protected:
  void read_sysex();
  KString * read_current_string(int obj_type, byte pad);
  void send_sysex(const byte * const sysex, UInt32 bytes_to_send);
  void write_current_string(int obj_type, KString *kstr);
};

#endif /* MOCK_KRONOS_H */
