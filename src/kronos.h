#ifndef KRONOS_H
#define KRONOS_H

#include <CoreMIDI/MIDIServices.h>
#include "set_list.h"
#include "midi_data.h"
#include "kstring.h"
#include "byte_data.h"

using namespace std;

typedef unsigned char byte;

enum SysexState {
  idle,
  waiting,
  receiving,
  received,
  error
};

class Kronos {
public:

  Kronos(byte channel);
  ~Kronos();

  void set_input(MIDIEndpointRef input_ref) { input = input_ref; }
  void set_output(MIDIEndpointRef output_ref) { output = output_ref; }

  virtual void receive_midi(const MIDIPacketList *pktlist);

  void read_current_set_list();
  void write_current_set_list();

  KString * read_current_slot_name();
  KString * read_current_slot_comments();
  virtual void write_current_slot_name(KString *kstr);
  virtual void write_current_slot_comments(KString *kstr);

  bool error_reply_seen();
  const char * const error_reply_message();

  void dump_sysex(const char * const msg);

protected:
  MIDIEndpointRef input;
  MIDIEndpointRef output;
  byte channel;
  SysexState sysex_state;
  ByteData sysex;
  byte obj_dump_bank;           // curr slot name/comment set list number
  int obj_dump_index;           // curr slot name/comment slot number

  virtual void read_sysex();
  virtual KString * read_current_string(int obj_type, byte pad);
  virtual void send_sysex(const byte * const sysex, UInt32 bytes_to_send);
  virtual void write_current_string(int obj_type, KString *kstr);
};

#endif /* KRONOS_H */
