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

#define UNDEFINED (-1)

#define SYSEX_HEADER SYSEX, KORG_MANUFACTURER_ID, static_cast<byte>(0x30 + channel), KRONOS_DEVICE_ID

#define SLEEP_MICROSECONDS 10000 // 10 milliseconds in microseconds
#define SYSEX_START_TIMEOUT_SECS 5
#define TIMEOUT_ERROR_REPLY 100

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
Kronos::Kronos(byte chan, RtMidiOut *output_port)
  : channel(chan), output(output_port), waiting_for_sysex_function(UNDEFINED)
{
  kronos_instance = this;
}

Kronos::~Kronos() {
  if (kronos_instance == this)
    kronos_instance = nullptr;
  close();
}

void Kronos::close() {
}

// ================ sysex I/O ================

bool Kronos::send_sysex(vector<byte> &sysex_bytes) {
  clog << "sending sysex, func "
       << setw(2) << setfill('0') << hex << (int)sysex_bytes[4]
       << "\n";
  if (output != nullptr)        // only null during testing
    output->sendMessage(&sysex_bytes);
  return true;
}

bool Kronos::message_is_wanted(vector<byte> *message) {
  return message->size() > 0
    && message->at(0) == SYSEX
    && (message->at(4) == waiting_for_sysex_function || message->at(4) == FUNC_CODE_REPLY);
}


// Copies incoming MIDI message to sysex if it's what we're looking for.
void Kronos::receive_midi(vector<byte> *message) {
  if (message_is_wanted(message))
    sysex = *message;
}

// Poll, waiting for next Kronos sysex message that matches `reply_function`
// to be read into `sysex`.
bool Kronos::read_sysex(const char * const func_name, byte reply_function) {
  clog << "reading sysex, waiting for reply func "
       << setw(2) << setfill('0') << hex << (int)reply_function
       << "\n";
  sysex.clear();
  waiting_for_sysex_function = reply_function; // tells callback to copy these
  time_t start = time(0);
  while (true) {
    if (message_is_wanted(&sysex))
      break;
    usleep(SLEEP_MICROSECONDS);
    if ((time(0) - start) >= SYSEX_START_TIMEOUT_SECS) {
      cerr << "Kronos::" << func_name << ": timeout waiting for sysex\n";
      waiting_for_sysex_function = UNDEFINED;
      return false;
    }
  }
  waiting_for_sysex_function = UNDEFINED;
  return true;
}

bool Kronos::get(vector<byte> &request_sysex, const char * const func_name, byte reply_function) {
  if (!send_sysex(request_sysex))
    return false;
  if (!read_sysex(func_name, reply_function))
    return false;
  if (error_reply_seen()) {
    cerr << "Kronos::" << func_name << " received an error response: " << error_reply_message() << "\n";
    return false;
  }
  return true;
}

void Kronos::send_channel_message(byte status, byte data1, byte data2) {
  vector<byte> message;
  message.push_back(status);
  message.push_back(data1);
  message.push_back(data2);
  if (output != nullptr)        // only null during testing
    output->sendMessage(&message);
}

// ================ error detection ================

bool Kronos::error_reply_seen() {
  return sysex.size() > 0 && sysex[0] == SYSEX && sysex[4] == FUNC_CODE_REPLY && sysex[5] > 0;
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
  vector<byte> request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP_REQ, static_cast<byte>(obj_type),
    EOX
  };
  if (!get(request_sysex, "read_current_string", FUNC_CODE_CURR_OBJ_DUMP))
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

// Returns true on success, false on error.
bool Kronos::read_set_list(int n, SetList &set_list) {
  if (!set_mode(mode_set_list))
    return false;

  goto_set_list(n);

  vector<byte> request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP_REQ, static_cast<byte>(OBJ_TYPE_SET_LIST),
    EOX
  };
  if (!get(request_sysex, "read_set_list", FUNC_CODE_CURR_OBJ_DUMP))
    return false;

  int start = 7;
  int end = start;
  while (sysex[end] != EOX) ++end;

  MIDIData midi_data(MD_INIT_MIDI, &sysex.data()[start], end - start);
  memcpy((void *)&set_list, (void *)midi_data.internal_bytes, midi_data.internal_len);
  return true;
}

SetList * Kronos::read_current_set_list() {
  vector<byte> request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP_REQ, static_cast<byte>(OBJ_TYPE_SET_LIST),
    EOX
  };
  if (!send_sysex(request_sysex))
    return 0;
  if (!read_sysex("read_current_set_list", FUNC_CODE_CURR_OBJ_DUMP))
    return 0;
  if (error_reply_seen()) {
    cerr << "sysex error response: " << error_reply_message() << "\n";
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
  vector<byte> request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP, static_cast<byte>(obj_type), 0
  };
  for (int i = 0; i < kstr->midi_len; ++i)
    request_sysex.push_back(kstr->midi_bytes[i]);
  request_sysex.push_back(EOX);

  get(request_sysex, "write_current_string", FUNC_CODE_REPLY);
}

void Kronos::write_current_slot_name(KString *kstr) {
  write_current_string(OBJ_TYPE_SET_LIST_SLOT_NAME, kstr);
}

void Kronos::write_current_slot_comments(KString *kstr) {
  write_current_string(OBJ_TYPE_SET_LIST_SLOT_COMMENTS, kstr);
}

bool Kronos::write_set_list(int n, SetList &set_list) {
  if (!set_mode(mode_set_list))
    return false;

  goto_set_list(n);

  MIDIData midi_data(MD_INIT_INTERNAL, (byte *)&set_list, sizeof(SetList));

  vector<byte> request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP, static_cast<byte>(OBJ_TYPE_SET_LIST),
    (byte)0,                                   // bank
    (byte)((n >> 7) & 0x7f), (byte)(n & 0x7f), // idH, idL
    0                                          // version
  };
  for (int i = 0; i < midi_data.midi_len; ++i)
    request_sysex.push_back(midi_data.midi_bytes[i]);
  request_sysex.push_back(EOX);

  bool got = get(request_sysex, "write_current_set_list", FUNC_CODE_REPLY);
  if (!got)
    return false;

  save_current_set_list();
  return true;
}

// ================ saving objects to non-volatile storage ================

void Kronos::save_current_set_list() {
  vector<byte> request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_STORE_BANK_REQ,
    static_cast<byte>(OBJ_TYPE_SET_LIST), 0,
    EOX
  };

  get(request_sysex, "save_current_set_list", FUNC_CODE_REPLY);
}

// ================ mode and movement commands ================

KronosMode Kronos::mode() {
  vector<byte> request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_MODE_REQ, EOX
  };
  if (!get(request_sysex, "mode", FUNC_CODE_MODE_DATA))
    return mode_combination;    // what else should I do?
  return (KronosMode)(sysex[5] & 0x0f);
}

bool Kronos::set_mode(KronosMode mode) {
  vector<byte> request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_MODE_CHANGE, (byte)mode, EOX
  };
  return get(request_sysex, "set_mode", FUNC_CODE_REPLY);
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
