#ifndef KRONOS_H
#define KRONOS_H

#include <CoreMIDI/MIDIServices.h>
#include "set_list.h"
#include "midi_data.h"
#include "kstring.h"

using namespace std;

typedef unsigned char byte;

class Kronos {
public:
  byte channel;
  SetList set_lists[128];

  Kronos(byte channel);
  ~Kronos();

  void set_input(MIDIEndpointRef input_ref) { input = input_ref; }
  void set_output(MIDIEndpointRef output_ref) { output = output_ref; }

  void receive_midi(const MIDIPacketList *pktlist);

  void read_all_set_lists();
  MIDIData * read_set_list(int set_list_num);

  KString * read_current_slot_name();
  KString * read_current_slot_comments();
  void write_current_slot_name(KString *kstr);
  void write_current_slot_comments(KString *kstr);

  bool error_reply_seen();
  const char * const error_reply_message();

  MIDIData * read_object_dump(byte type, byte bank, int index);

  void dump_sysex(const char * const msg);

private:
  MIDIEndpointRef input;
  MIDIEndpointRef output;
  bool receiving_sysex;
  byte *sysex;
  size_t sysex_allocated_size;
  size_t sysex_length;

  void read_sysex();
  KString * read_current_string(int obj_type, byte pad);
  void send_sysex(const byte * const sysex, UInt32 bytes_to_send);
  void write_current_string(int obj_type, KString *kstr);

  void clear_sysex_buffer();
  void append_sysex_byte(byte b);
};

#endif /* KRONOS_H */
