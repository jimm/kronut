#pragma once

#include <rtmidi/RtMidi.h>
#include "types.h"
#include "consts.h"
#include "set_list.h"
#include "midi_data.h"
#include "kstring.h"

using namespace std;

enum KronosMode {
  mode_combination = MODE_COMBINATION,
  mode_program = MODE_PROGRAM,
  mode_sequencer = MODE_SEQUENCER,
  mode_sampling = MODE_SAMPLING,
  mode_global = MODE_GLOBAL,
  mode_disk = MODE_DISK,
  mode_set_list = MODE_SET_LIST,
};

class Kronos {
public:

  Kronos(byte channel, RtMidiOut *output_port);
  virtual ~Kronos();

  void close();

  int setlist() { return setlist_num; }
  int slot() { return slot_num; }
  void set_setlist(int n) { setlist_num = n; }
  void set_slot(int n) { slot_num = n; }

  bool message_is_wanted(message *message);

  // Callback that receives MIDI messages.
  void receive_midi(message *message);

  // For text editing via TextEditor
  SetList * read_current_set_list();

  // For file import/export via FileEditor
  bool read_set_list(SetList &set_list);
  bool write_set_list(SetList &set_list);

  KString * read_current_slot_name();
  KString * read_current_slot_comments();
  virtual void write_current_slot_name(KString *kstr);
  virtual void write_current_slot_comments(KString *kstr);

  void save_current_set_list();

  void goto_set_list(int n);
  KronosMode mode();
  bool set_mode(KronosMode mode);

  bool error_reply_seen();
  const char * const error_reply_message();

  void dump_sysex(const char * const msg);

protected:
  RtMidiOut *output;
  int setlist_num;
  int slot_num;
  byte channel;
  byte waiting_for_sysex_function;
  message sysex;

  virtual bool send_sysex(const char * const func_name, message &sysex);
  virtual bool read_sysex(const char * const func_name, byte reply_function);
  virtual bool call_sysex_func(message &request_sysex, const char * const func_name, byte reply_function);
  virtual void send_channel_message(byte status, byte data1, byte data2);

  virtual KString * read_current_string(int obj_type, byte pad);
  virtual void write_current_string(int obj_type, KString *kstr);
};
