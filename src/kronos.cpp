#include <CoreMIDI/MIDIServices.h>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "consts.h"
#include "kronos.h"
#include "midi_data.h"
#include "kstring.h"
#include "set_list.h"
#include "utils.h"

#define SYSEX_HEADER SYSEX, KORG_MANUFACTURER_ID, static_cast<byte>(0x30 + channel), KRONOS_DEVICE_ID
#define MIDI_BUFSIZ 1024
#define SYSEX_BUF_EVENTS 1024

#define SYSEX_START_TIMEOUT_SECS 5
#define SYSEX_READ_TIMEOUT_SECS 60
#define TIMEOUT_ERROR_REPLY 100
// 0.001 seconds in nanoseconds
#define WAIT_NANOSECS 1000000
#define START_WAIT_TIMES (1000 * SYSEX_START_TIMEOUT_SECS)
#define TIMEOUT_WAIT_TIMES (1000 * SYSEX_READ_TIMEOUT_SECS)

#define is_realtime(b) ((b) >= CLOCK)
#define is_status(b) ((b) >= NOTE_OFF)

static const char * const error_format = "Kronos::%s sysex error response: %s\n";
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

static void midi_read_proc(const MIDIPacketList *pktlist, void *ref_con, void *conn_ref_con) {
  Kronos *kronos = (Kronos *)ref_con;
  const MIDIPacket *packet = &pktlist->packet[0];
  for (unsigned int i = 0; i < pktlist->numPackets; ++i) {
    kronos->receive_midi_packet(packet);
    packet = MIDIPacketNext(packet);
  }
}

Kronos *Kronos_instance() {
  return kronos_instance;
}

// ================ allocation ================

// chan must be 0-15
Kronos::Kronos(byte chan, MIDIClientRef client, int input_device_num, int output_device_num)
  : channel(chan), receive_state(idle)
{
  in_endpoint = MIDIGetSource(input_device_num);
  MIDIInputPortCreate(client, CFSTR("Kronos Input Port"), midi_read_proc, (void*)this, &in_port);
  MIDIPortConnectSource(in_port, in_endpoint, 0);

  out_endpoint = MIDIGetDestination(output_device_num);
  MIDIOutputPortCreate(client, CFSTR("Kronos Output Port"), &out_port);

  kronos_instance = this;
}

Kronos::~Kronos() {
  if (kronos_instance == this)
    kronos_instance = nullptr;
  if (in_port != 0)
    MIDIPortDispose(in_port);
  if (in_endpoint != 0)
    MIDIEndpointDispose(in_endpoint);
  if (out_port != 0)
    MIDIPortDispose(out_port);
  if (out_endpoint != 0)
    MIDIEndpointDispose(out_endpoint);
}

// ================ sysex I/O ================

void midi_completion_proc(MIDISysexSendRequest *_request) {
  cout << "done sending sysex message" << endl; // DEBUG
}

bool Kronos::send_sysex(const byte * const sysex_bytes, const int num_bytes) {
  MIDISysexSendRequest request = {
    out_endpoint,               // MIDIEndpointRef
    sysex_bytes,                // const Byte *
    (UInt32)num_bytes,          // UInt32
    false,                      // Boolean
    {0, 0, 0},                  // Byte[3] reserved
    midi_completion_proc,       // MIDICompletionProc
    0                           // void *
  };

  sysex.clear();
  receive_state = waiting;
  OSStatus status = MIDISendSysex(&request);
  return status == 0;
}

// Wait for next System Exclusive message to be read into `sysex`. We first
// receive and dump everything as quickly as we can into a ByteData. Then we
// post-process it, removing realtime bytes and any bytes before or after
// the sysex.
void Kronos::receive_midi_packet(const MIDIPacket *packet) {
  if (receive_state == idle)
    return;

  for (unsigned int i = 0; i < packet->length && receive_state != received; ++i) {
    byte b = packet->data[i];
    switch (b) {
    case SYSEX:
      receive_state = receiving;
      sysex.clear();
      sysex.append(b);
      break;
    case EOX:
      sysex.append(b);
      receive_state = received;
      break;
    default:
      if (receive_state == receiving && !is_realtime(b)) {
        if (is_status(b)) {    // we ignore the status byte
          b = EOX;
          receive_state = received;
        }
        sysex.append(b);
      }
      break;
    }
  }
}

bool Kronos::wait_for_sysex() {
  int i;
  struct timespec rqtp = {0, WAIT_NANOSECS};

  cout << "starting wait_for_sysex" << endl;
  for (i = 0; i < START_WAIT_TIMES; ++i) {
    SysexState s = receive_state;
    if (s == receiving || s == received)
      break;
    nanosleep(&rqtp, 0);
  }
  if (i >= START_WAIT_TIMES) {
    cerr << "timeout waiting for sysex" << endl;
    return false;
  }

  for (i = 0; i < TIMEOUT_WAIT_TIMES; ++i) {
    SysexState s = receive_state;
    if (s == received)
      break;
    nanosleep(&rqtp, 0);
  }
  if (i >= TIMEOUT_WAIT_TIMES) {
    cerr << "timeout waiting for end of sysex" << endl;
    return false;
  }

  receive_state = idle;
  return true;
}

bool Kronos::get(const byte * const request_sysex, const int request_num_bytes, const char * const func_name) {
  if (!send_sysex(request_sysex, request_num_bytes))
    return false;
  if (!wait_for_sysex())
    return false;
  if (error_reply_seen()) {
    cerr << "Kronos::" << func_name << " received an error response: " << error_reply_message() << endl;
    return false;
  }
  return true;
}

void Kronos::send_channel_message(byte status, byte data1, byte data2) {
  MIDIPacketList pktlist;
  pktlist.numPackets = 1;
  pktlist.packet[0] = {0, 3, {status, data1, data2}};
  MIDISend(out_port, out_endpoint, &pktlist);
}

// ================ error detection ================

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

// ================ reading objects ================

// Returns a newly allocated KString.
KString * Kronos::read_current_string(int obj_type, byte pad) {
  const byte request_sysex[] = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP_REQ, static_cast<byte>(obj_type),
    EOX
  };
  if (!get(request_sysex, (int)sizeof(request_sysex), "read_current_string"))
    return 0;

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

void Kronos::read_set_list(int n, SetList &set_list) {
  set_mode(mode_set_list);
  goto_set_list(n);

  const byte request_sysex[] = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP_REQ, static_cast<byte>(OBJ_TYPE_SET_LIST),
    EOX
  };
  if (!get(request_sysex, (int)sizeof(request_sysex), "read_set_list"))
    return;

  int start = 7;
  int end = start;
  while (sysex[end] != EOX) ++end;

  MIDIData midi_data(MD_INIT_MIDI, &sysex.data()[start], end - start);
  memcpy((void *)&set_list, (void *)midi_data.internal_bytes, midi_data.internal_len);
}

SetList * Kronos::read_current_set_list() {
  const byte request_sysex[] = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP_REQ, static_cast<byte>(OBJ_TYPE_SET_LIST),
    EOX
  };
  if (!send_sysex(request_sysex, 7))
    return 0;
  if (!wait_for_sysex())
    return 0;
  if (error_reply_seen()) {
    fprintf(stderr, "sysex error response: %s\n", error_reply_message());
    return 0;
  }

  int start = 7;
  int end = start;
  while (sysex[end] != EOX) ++end;

  MIDIData midi_data(MD_INIT_MIDI, &sysex.data()[start], end - start);
  SetList *set_list = new SetList();
  memcpy((void *)set_list, (void *)midi_data.internal_bytes, midi_data.internal_len);
  return set_list;
}

// ================ writing objects ================

void Kronos::write_current_string(int obj_type, KString *kstr) {
  byte request_sysex[kstr->midi_len + 8];
  const byte request_sysex_header[] = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP, static_cast<byte>(obj_type), 0
  };

  memcpy(request_sysex, request_sysex_header, 7); // start of sysex
  memcpy(request_sysex + 7, kstr->midi_bytes, kstr->midi_len);
  request_sysex[7 + kstr->midi_len] = EOX;  // end of sysex

  if (!get(request_sysex, 8 + kstr->midi_len, "write_current_string"))
    return;
}

void Kronos::write_current_slot_name(KString *kstr) {
  write_current_string(OBJ_TYPE_SET_LIST_SLOT_NAME, kstr);
}

void Kronos::write_current_slot_comments(KString *kstr) {
  write_current_string(OBJ_TYPE_SET_LIST_SLOT_COMMENTS, kstr);
}

void Kronos::write_set_list(int n, SetList &set_list) {
  set_mode(mode_set_list);
  goto_set_list(n);

  MIDIData midi_data(MD_INIT_INTERNAL, (byte *)&set_list, sizeof(SetList));

  const byte request_sysex_header[] = {
    SYSEX_HEADER,
    FUNC_CODE_OBJ_DUMP, static_cast<byte>(OBJ_TYPE_SET_LIST),
    (byte)0,                                   // bank
    (byte)((n >> 7) & 0x7f), (byte)(n & 0x7f), // idH, idL
    0                                          // version
  };
  size_t header_size = sizeof(request_sysex_header);
  byte request_sysex[header_size + midi_data.midi_len + 1];

  memcpy(request_sysex, request_sysex_header, header_size); // header
  memcpy(request_sysex + header_size, midi_data.midi_bytes, midi_data.midi_len); // data
  request_sysex[header_size + midi_data.midi_len] = EOX; // end of sysex

  if (!get(request_sysex, (int)sizeof(request_sysex), "write_current_set_list"))
    return;

  save_current_set_list();
}

// ================ saving objects to non-volatile storage ================

void Kronos::save_current_set_list() {
  const byte request_sysex[] = {
    SYSEX_HEADER,
    FUNC_CODE_STORE_BANK_REQ,
    static_cast<byte>(OBJ_TYPE_SET_LIST), 0,
    EOX
  };

  if (!get(request_sysex, (int)sizeof(request_sysex), "save_current_set_list"))
    return;
}

// ================ mode and movement commands ================

KronosMode Kronos::mode() {
  const byte request_sysex[] = {
    SYSEX_HEADER,
    FUNC_CODE_MODE_REQ, EOX
  };
  if (!get(request_sysex, (int)sizeof(request_sysex), "mode"))
    return mode_combination;    // what else should I do?
  return (KronosMode)(sysex[5] & 0x0f);
}

void Kronos::set_mode(KronosMode mode) {
  const byte request_sysex[] = {
    SYSEX_HEADER,
    FUNC_CODE_MODE_CHANGE, (byte)mode, EOX
  };
  get(request_sysex, (int)sizeof(request_sysex), "set_mode");
}

void Kronos::goto_set_list(int n) {
  send_channel_message(CONTROLLER + channel, CC_BANK_SELECT_MSB, 0);
  send_channel_message(CONTROLLER + channel, CC_BANK_SELECT_LSB, (byte)(n & 0xff));
  send_channel_message(PROGRAM_CHANGE + channel, 0, 0);
}

// ================ helpers ================

void Kronos::dump_sysex(const char * const msg) {
  dump_hex(sysex.data(), sysex.size(), msg);
}
