#ifndef KRONOS_H
#define KRONOS_H

#include <CoreMIDI/MIDIServices.h>
#include "consts.h"
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

enum KronosMode {
  mode_combination = MODE_COMBINATION,
  mode_program = MODE_PROGRAM,
  mode_sequencer = MODE_SEQUENCER,
  mode_sampling = MODE_SAMPLING,
  mode_global = MODE_GLOBAL,
  mode_disk = MODE_DISK,
  mode_set_list = MODE_SET_LIST
};

class Kronos {
public:

  Kronos(byte channel);
  ~Kronos();

  void set_output(MIDIPortRef output_port, MIDIEndpointRef output_ref);

  virtual void receive_midi(const MIDIPacketList *pktlist);

  void read_current_set_list(SetList &set_list);
  void write_current_set_list(SetList &set_list);

  KString * read_current_slot_name();
  KString * read_current_slot_comments();
  virtual void write_current_slot_name(KString *kstr);
  virtual void write_current_slot_comments(KString *kstr);

  void goto_set_list(byte n);
  KronosMode mode();
  void set_mode(KronosMode mode);

  bool error_reply_seen();
  const char * const error_reply_message();

  void dump_sysex(const char * const msg);

protected:
  MIDIPortRef output_port;
  MIDIEndpointRef output_ref;
  byte channel;
  SysexState sysex_state;
  ByteData sysex;

  virtual void read_sysex();
  virtual KString * read_current_string(int obj_type, byte pad);
  virtual void send_sysex(const byte * const sysex, UInt32 bytes_to_send);
  void send_channel_message(byte status, byte data1, byte data2);
  virtual void write_current_string(int obj_type, KString *kstr);

  void warn_if_error_reply(const char * const func_name);
};

#endif /* KRONOS_H */
