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

#define READ_SYSEX_TIMEOUT_SECS 5
#define TIMEOUT_ERROR_REPLY 100

#define is_realtime(b) ((b) >= CLOCK)

static Kronos *kronos_instance;

static const char * const error_reply_messages[] = {
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
  "(unknown error code)",
  // The following errors are kronut errors, not Kronos errors
  "timeout"
};

Kronos *Kronos_instance() {
  return kronos_instance;
}

// ================ allocation ================

// chan must be 0-15
Kronos::Kronos(byte chan) : channel(chan), sysex_state(idle) {
  kronos_instance = this;
}

Kronos::~Kronos() {
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
        sysex_state = receiving;
        sysex.clear();
        sysex.append(b);
        break;
      case EOX:
        sysex.append(b);
        sysex_state = received;
        break;
      default:
        if (sysex_state == receiving && !is_realtime(b))
          sysex.append(b);
        break;
      }
    }
    packet = MIDIPacketNext(packet);
  }
}

void Kronos::send_sysex(const byte * const sysex_bytes, const UInt32 bytes_to_send) {
  // In anticipation of receiving a response (the Kronos always sends a
  // response), we clear the sysex buffer and start waiting.
  sysex.clear();
  sysex_state = waiting;

  MIDISysexSendRequest req;

  req.destination = output;
  req.data = sysex_bytes;
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
  while (sysex_state != received && sysex_state != error)
    ;
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
  // Kronut errors
  case TIMEOUT_ERROR_REPLY: error_index = 12; break;
  // anything else
  default: error_index = 11; break;
  }
  return error_reply_messages[error_index];
}

// Returns a newly allocated KString.
KString * Kronos::read_current_string(int obj_type, byte pad) {
  const byte request_sysex[] = {
    SYSEX, KORG_MANUFACTURER_ID,
    static_cast<byte>(0x30 + channel), KRONOS_DEVICE_ID,
    FUNC_CODE_CURR_OBJ_DUMP_REQ, static_cast<byte>(obj_type),
    EOX
  };
  send_sysex(request_sysex, sizeof(request_sysex));
  read_sysex();
  if (error_reply_seen())
    fprintf(stderr, "sysex error response: %s\n", error_reply_message());

  int start = 7;
  int end = start;
  while (sysex[end] != EOX) ++end;
  return new KString(MD_INIT_MIDI, sysex.data() + start, end - start, pad);
}

// Returns a newly allocated KString.
KString * Kronos::read_current_slot_name() {
  return read_current_string(OBJ_TYPE_SET_LIST_SLOT_NAME, 0);
}

// Returns a newly allocated KString.
KString * Kronos::read_current_slot_comments() {
  return read_current_string(OBJ_TYPE_SET_LIST_SLOT_COMMENTS, 0);
}

void Kronos::read_current_set_list(SetList &set_list) {
  const byte request_sysex[] = {
    SYSEX, KORG_MANUFACTURER_ID,
    static_cast<byte>(0x30 + channel), KRONOS_DEVICE_ID,
    FUNC_CODE_CURR_OBJ_DUMP_REQ, static_cast<byte>(OBJ_TYPE_SET_LIST),
    EOX
  };
  send_sysex(request_sysex, sizeof(request_sysex));
  read_sysex();
  if (error_reply_seen())
    fprintf(stderr, "sysex error response: %s\n", error_reply_message());

  int start = 7;
  int end = start;
  while (sysex[end] != EOX) ++end;

  MIDIData midi_data(MD_INIT_MIDI, &sysex.data()[start], end - start);
  memcpy((void *)&set_list, (void *)midi_data.internal_bytes, midi_data.internal_len);
}

void Kronos::write_current_string(int obj_type, KString *kstr) {
  byte request_sysex[kstr->midi_len + 8];
  const byte request_sysex_header[] = {
    SYSEX, KORG_MANUFACTURER_ID,
    static_cast<byte>(0x30 + channel), KRONOS_DEVICE_ID,
    FUNC_CODE_CURR_OBJ_DUMP, static_cast<byte>(obj_type), 0
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

void Kronos::write_current_set_list(SetList &set_list) {
  MIDIData midi_data(MD_INIT_INTERNAL, (byte *)&set_list, sizeof(SetList));

  byte request_sysex[midi_data.midi_len + 8];
  const byte request_sysex_header[] = {
    SYSEX, KORG_MANUFACTURER_ID,
    static_cast<byte>(0x30 + channel), KRONOS_DEVICE_ID,
    FUNC_CODE_CURR_OBJ_DUMP, static_cast<byte>(OBJ_TYPE_SET_LIST), 0
  };

  memcpy(request_sysex, request_sysex_header, 7); // start of sysex
  memcpy(request_sysex + 7, midi_data.midi_bytes, midi_data.midi_len);
  request_sysex[7 + midi_data.midi_len] = EOX;  // end of sysex

  send_sysex(request_sysex, sizeof(request_sysex));
  read_sysex();
  if (error_reply_seen())
    fprintf(stderr, "sysex error response: %s\n", error_reply_message());
}

void Kronos::dump_sysex(const char * const msg) {
  dump_hex(sysex.data(), sysex.size(), msg);
}
