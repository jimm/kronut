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

enum KronosMode {
  mode_combination = MODE_COMBINATION,
  mode_program = MODE_PROGRAM,
  mode_sequencer = MODE_SEQUENCER,
  mode_sampling = MODE_SAMPLING,
  mode_global = MODE_GLOBAL,
  mode_disk = MODE_DISK,
  mode_set_list = MODE_SET_LIST
};

enum SysexState {
  idle,                         // do not expect or need sysex
  waiting,                      // expecting sysex
  receiving,
  received,
  error
};

class Kronos {
public:

  Kronos(byte channel, MIDIClientRef client, int input_device_num, int output_device_num);
  ~Kronos();

  virtual void receive_midi_packet(const MIDIPacket *packet);

  // For text editing via TextEditor
  SetList * read_current_set_list();

  // For file import/export via FileEditor
  void read_set_list(int n, SetList &set_list);
  void write_set_list(int n, SetList &set_list);

  KString * read_current_slot_name();
  KString * read_current_slot_comments();
  virtual void write_current_slot_name(KString *kstr);
  virtual void write_current_slot_comments(KString *kstr);

  void save_current_set_list();

  void goto_set_list(int n);
  KronosMode mode();
  void set_mode(KronosMode mode);

  bool error_reply_seen();
  const char * const error_reply_message();

  void dump_sysex(const char * const msg);

protected:
  MIDIEndpointRef in_endpoint;
  MIDIPortRef in_port;
  MIDIEndpointRef out_endpoint;
  MIDIPortRef out_port;
  byte channel;
  ByteData sysex;
  SysexState receive_state;

  virtual bool send_sysex(const byte * const sysex, const int num_bytes);
  virtual bool wait_for_sysex();
  virtual bool get(const byte * const request_sysex, const int request_num_bytes, const char * const func_name);
  virtual void send_channel_message(byte status, byte data1, byte data2);

  virtual KString * read_current_string(int obj_type, byte pad);
  virtual void write_current_string(int obj_type, KString *kstr);
};

#endif /* KRONOS_H */
