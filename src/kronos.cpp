#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "consts.h"
#include "kronos.h"
#include "midi_data.h"
#include "kstring.h"
#include "set_list.h"
#include "utils.h"

#define SYSEX_CHUNK_SIZE 1024

static Kronos *kronos_instance;

static const char * const error_reply_messages[12] = {
  "no error",
  "parameter type specified is incorrect for current mode",
  "unknown param message type, unknown parameter id or index",
  "short or otherwise mangled message",
  "target object not found",
  "insufficient resources to complete request",
  "parameter value is out of range",
  "(internal error code)",
  "other error: program bank is wrong type for received program dump (Func 73, 75); invalid data in Preset Pattern Dump (Func 7B).",
  "target object is protected",
  "memory overflow",
  "(unknown error code)"
};

Kronos *Kronos_instance() {
  return kronos_instance;
}

// ================ allocation ================

// channel 1-15
Kronos::Kronos(byte chan) : channel(chan-1), receiving_sysex(false) {
  sysex = (byte *)malloc(SYSEX_CHUNK_SIZE);
  sysex_allocated_size = SYSEX_CHUNK_SIZE;
  sysex_length = 0;
  kronos_instance = this;
}

Kronos::~Kronos() {
  free(sysex);
  if (kronos_instance == this)
    kronos_instance = 0;
}

void Kronos::receive_midi(const MIDIPacketList *packet_list) {
  const MIDIPacket *packet = &packet_list->packet[0];
  for (int i = 0; i < packet_list->numPackets; ++i) {
    for (int j = 0; j < packet->length; ++j) {
      byte b = packet->data[j];
      switch (b) {
      case SYSEX:
        clear_sysex_buffer();
        receiving_sysex = true;
        append_sysex_byte(b);
        break;
      case EOX:
        receiving_sysex = false;
        append_sysex_byte(b);
        break;
      default:
        if (receiving_sysex)
          append_sysex_byte(b);
        break;
      }
    }
    packet = MIDIPacketNext(packet);
  }
}

void Kronos::clear_sysex_buffer() {
  sysex_length = 0;
  receiving_sysex = false;
}

void Kronos::append_sysex_byte(byte b) {
  if (sysex_length == sysex_allocated_size) {
    sysex_allocated_size += sysex_allocated_size;
    sysex = (byte *)realloc(sysex, sysex_allocated_size);
  }
  sysex[sysex_length++] = b;
}

void Kronos::send_sysex(const byte * const sysex, const UInt32 bytes_to_send) {
  // In anticipation of receiving a response (the Kronos always sends a
  // response), we clear the sysex buffer.
  clear_sysex_buffer();

  MIDISysexSendRequest req;

  req.destination = output;
  req.data = sysex;
  req.bytesToSend = bytes_to_send;
  req.complete = false;
  req.completionProc = 0;
  req.completionRefCon = 0;

  while (req.bytesToSend > 0) {
    OSStatus result = MIDISendSysex(&req);
    if (result != 0) {
      fprintf(stderr, "MIDISendSysex error: %d\n", result);
      return;
    }
  }
}

// Wait for next System Exclusive message to be read into `sysex`.
void Kronos::read_sysex() {
  // TODO time out
  usleep(1);                   // segfault without this --- why?
  while (sysex_length == 0)
    usleep(10);
  usleep(1);
  while (receiving_sysex)
    usleep(10);
  usleep(1);                   // segfault without this --- why?
}

bool Kronos::error_reply_seen() {
  return sysex[4] == FUNC_CODE_REPLY && sysex[5] > 0;
}

const char * const Kronos::error_reply_message() {
  int error_index;
  switch (sysex[5]) {
  case 0: error_index = 0; break;
  case 1: error_index = 1; break;
  case 2: error_index = 2; break;
  case 3: error_index = 3; break;
  case 4: error_index = 4; break;
  case 5: error_index = 5; break;
  case 6: error_index = 6; break;
  case 7: error_index = 7; break;
  case 64: error_index = 8; break;
  case 65: error_index = 9; break;
  case 66: error_index = 10; break;
  default: error_index = 11; break;
  }
  return error_reply_messages[error_index];
}

// Returns a newly allocated KString.
KString * Kronos::read_current_string(int obj_type, byte pad) {
  const byte request_sysex[] = {
    SYSEX, KORG_MANUFACTURER_ID, 0x30 + 0, KRONOS_DEVICE_ID,
    FUNC_CODE_CURR_OBJ_DUMP_REQ, obj_type,
    EOX
  };
  send_sysex(request_sysex, sizeof(request_sysex));
  read_sysex();
  if (error_reply_seen())
    fprintf(stderr, "sysex error response: %s\n", error_reply_message());

  set_obj_dump_bank_and_index();
  int start = 7;
  int end = start;
  while (sysex[end] != EOX) ++end;
  return new KString(MD_INIT_MIDI, sysex + start, end - start, pad);
}

// Returns a newly allocated KString.
KString * Kronos::read_current_slot_name() {
  return read_current_string(OBJ_TYPE_SET_LIST_SLOT_NAME, 0);
}

// Returns a newly allocated KString.
KString * Kronos::read_current_slot_comments() {
  return read_current_string(OBJ_TYPE_SET_LIST_SLOT_COMMENTS, 0);
}

void Kronos::write_current_string(int obj_type, KString *kstr) {
  byte request_sysex[kstr->midi_len + 8];
  const byte request_sysex_header[] = {
    SYSEX, KORG_MANUFACTURER_ID, 0x30 + 0, KRONOS_DEVICE_ID,
    FUNC_CODE_CURR_OBJ_DUMP, obj_type, 0
  };

  memcpy(request_sysex, request_sysex_header, 7); // start of sysex
  memcpy(request_sysex + 7, kstr->midi_bytes, kstr->midi_len);
  request_sysex[7 + kstr->midi_len] = EOX;  // end of sysex

  send_sysex(request_sysex, sizeof(request_sysex));
  read_sysex();
  if (error_reply_seen())
    fprintf(stderr, "sysex error response: %s\n", error_reply_message());
}

void Kronos::write_current_slot_name(KString *kstr) {
  write_current_string(OBJ_TYPE_SET_LIST_SLOT_NAME, kstr);
}

void Kronos::write_current_slot_comments(KString *kstr) {
  write_current_string(OBJ_TYPE_SET_LIST_SLOT_COMMENTS, kstr);
}

// Reads object dump into sysex, assumed to be correct size.
MIDIData * Kronos::read_object_dump(byte type, byte bank, int index) {
  const byte request_sysex[] = {
    SYSEX, KORG_MANUFACTURER_ID, 0x30 + channel, KRONOS_DEVICE_ID,
    FUNC_CODE_OBJ_DUMP_REQ, type, bank, (index >> 7) & 0x7f,
    index & 0x7f, EOX
  };
  send_sysex(request_sysex, sizeof(request_sysex));
  read_sysex();
  if (error_reply_seen())
    fprintf(stderr, "sysex error response: %s\n", error_reply_message());

  set_obj_dump_bank_and_index();
  int start = 10;
  int end = start;
  while (sysex[end] != EOX) ++end;
  return new MIDIData(MD_INIT_MIDI, sysex + start, end - start);
}

void Kronos::set_obj_dump_bank_and_index() {
  obj_dump_bank = sysex[6];
  obj_dump_index = (sysex[7] << 7) + sysex[8];
}

void Kronos::dump_sysex(const char * const msg) {
  dump_hex(sysex, sysex_length, msg);
}
