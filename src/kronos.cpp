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
#define UNKNOWN_ERROR_BUFFER_LEN 128


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
static char unknown_error_buf[UNKNOWN_ERROR_BUFFER_LEN];

Kronos *Kronos_instance() {
  return kronos_instance;
}

// ================ allocation ================

// chan must be 0-15
Kronos::Kronos(byte chan, RtMidiOut *output_port)
  : channel(chan), output(output_port), setlist_num(UNDEFINED),
    slot_num(UNDEFINED), waiting_for_sysex_function(UNDEFINED)
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

bool Kronos::send_sysex(const char * const func_name, message &sysex_bytes) {
  clog << "Kronos::" << func_name << ": sending sysex, func "
       << HEXPRINT(2, sysex_bytes[4]) << "\n";
  if (output != nullptr)        // only null during testing
    output->sendMessage(&sysex_bytes);
  return true;
}

bool Kronos::message_is_wanted(message *message) {
  if (message->empty())
    return false;

  // It's a sysex; see if we want it
  byte status = message->at(0);
  if (status == SYSEX) {
    byte func_code = message->at(4);
    return func_code == waiting_for_sysex_function || func_code == FUNC_CODE_REPLY;
  }

  byte high_nibble = status & 0xf0;

  // We want program changes
  if (high_nibble == PROGRAM_CHANGE)
    return true;

  // We also want bank change LSB messages.
  if (high_nibble == CONTROLLER) {
    byte controller = message->at(1);
    if (controller == CC_BANK_SELECT_LSB)
      return true;
  }

  return false;
}


// Copies incoming MIDI message to sysex if it's what we're looking for.
void Kronos::receive_midi(message *message) {
  if (!message_is_wanted(message))
    return;

  byte status = message->at(0);
  byte data1 = message->at(1);

  if (status == SYSEX)
    sysex = *message;
  else {
    byte high_nibble = status & 0xf0;
    if (high_nibble == PROGRAM_CHANGE)
      slot_num = (int)data1;
    else if (high_nibble == CONTROLLER && data1 == CC_BANK_SELECT_LSB)
      setlist_num = (int)data1;
  }
}

// Poll, waiting for next Kronos sysex message that matches `reply_function`
// to be read into `sysex`.
bool Kronos::read_sysex(const char * const func_name, byte reply_function) {
  sysex.clear();
  waiting_for_sysex_function = reply_function; // tells callback to copy these
  clog << "Kronos::" << func_name << ": reading sysex, waiting for reply func "
       << HEXPRINT(2, reply_function) << "\n";
  time_t start = time(0);
  while (true) {
    if (error_reply_seen()) {
      cerr << "sysex error response: " << error_reply_message() << "\n";
      dump_sysex("ERROR REPLY SEEN IN WHILE TRUE");
      waiting_for_sysex_function = UNDEFINED;
      return false;
    }
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

bool Kronos::call_sysex_func(message &request_sysex, const char * const func_name, byte reply_function) {
  clog << "Kronos::call_sysex_func, func = " << func_name << ", reply_function = " << HEXPRINT(2, reply_function) << "\n";
  if (!send_sysex(func_name, request_sysex))
    return false;
  if (!read_sysex(func_name, reply_function))
    return false;
  return true;
}

void Kronos::send_channel_message(byte status, byte data1, byte data2) {
  message message;
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
  int error_code = (int)sysex[5];

  switch (error_code) {
  case 0: error_index = 0; break;
  case 1: error_index = 1; break;
  case 2: error_index = 2; break;
  case 3: error_index = 3; break;
  case 4: error_index = 4; break;
  case 5: error_index = 5; break;
  case 6: error_index = 6; break;
  case 7: error_index = 7; break;
  case 0x64: error_index = 8; break;
  case 0x65: error_index = 9; break;
  case 0x66: error_index = 10; break;
  default:
    snprintf(unknown_error_buf, UNKNOWN_ERROR_BUFFER_LEN,
             "unknown error reply code: %02x", error_code);
    return unknown_error_buf;
  }
  return error_reply_messages[error_index];
}

// ================ reading objects ================

// Returns a newly allocated KString.
KString * Kronos::read_current_string(int obj_type, byte pad) {
  message request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP_REQ, static_cast<byte>(obj_type),
    EOX
  };
  if (!call_sysex_func(request_sysex, "read_current_string", FUNC_CODE_CURR_OBJ_DUMP))
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
bool Kronos::read_set_list(SetList &set_list) {
  if (!set_mode(mode_set_list))
    return false;

  goto_set_list(setlist_num);

  message request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP_REQ, static_cast<byte>(OBJ_TYPE_SET_LIST),
    EOX
  };
  if (!call_sysex_func(request_sysex, "read_set_list", FUNC_CODE_CURR_OBJ_DUMP))
    return false;

  int start = 7;
  int end = start;
  while (sysex[end] != EOX) ++end;

  MIDIData midi_data(MD_INIT_MIDI, &sysex.data()[start], end - start);
  memcpy((void *)&set_list, (void *)midi_data.internal_bytes, midi_data.internal_len);
  return true;
}

SetList * Kronos::read_current_set_list() {
  message request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP_REQ, static_cast<byte>(OBJ_TYPE_SET_LIST),
    EOX
  };
  if (!call_sysex_func(request_sysex, "read_set_list", FUNC_CODE_CURR_OBJ_DUMP))
    return 0;

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
  message request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP, static_cast<byte>(obj_type), 0
  };
  for (int i = 0; i < kstr->midi_len; ++i)
    request_sysex.push_back(kstr->midi_bytes[i]);
  request_sysex.push_back(EOX);

  call_sysex_func(request_sysex, "write_current_string", FUNC_CODE_REPLY);
}

void Kronos::write_current_slot_name(KString *kstr) {
  write_current_string(OBJ_TYPE_SET_LIST_SLOT_NAME, kstr);
}

void Kronos::write_current_slot_comments(KString *kstr) {
  write_current_string(OBJ_TYPE_SET_LIST_SLOT_COMMENTS, kstr);
}

bool Kronos::write_set_list(SetList &set_list) {
  if (!set_mode(mode_set_list))
    return false;

  goto_set_list(setlist_num);

  MIDIData midi_data(MD_INIT_INTERNAL, (byte *)&set_list, sizeof(SetList));

  message request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_CURR_OBJ_DUMP, static_cast<byte>(OBJ_TYPE_SET_LIST),
    (byte)0,                                   // bank
    (byte)((setlist_num >> 7) & 0x7f), (byte)(setlist_num & 0x7f), // idH, idL
    0                                          // version
  };
  for (int i = 0; i < midi_data.midi_len; ++i)
    request_sysex.push_back(midi_data.midi_bytes[i]);
  request_sysex.push_back(EOX);

  bool got = call_sysex_func(request_sysex, "write_current_set_list", FUNC_CODE_REPLY);
  if (!got)
    return false;

  save_current_set_list();
  return true;
}

// ================ saving objects to non-volatile storage ================

void Kronos::save_current_set_list() {
  message request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_STORE_BANK_REQ,
    static_cast<byte>(OBJ_TYPE_SET_LIST), 0, // bank must be 0
    EOX
  };

  call_sysex_func(request_sysex, "save_current_set_list", FUNC_CODE_REPLY);
}

// ================ mode and movement commands ================

KronosMode Kronos::mode() {
  message request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_MODE_REQ, EOX
  };
  if (!call_sysex_func(request_sysex, "mode", FUNC_CODE_MODE_DATA))
    return mode_combination;    // what else should I do?
  return (KronosMode)(sysex[5] & 0x0f);
}

bool Kronos::set_mode(KronosMode mode) {
  message request_sysex = {
    SYSEX_HEADER,
    FUNC_CODE_MODE_CHANGE, (byte)mode, EOX
  };
  return call_sysex_func(request_sysex, "set_mode", FUNC_CODE_REPLY);
}

 void Kronos::goto_set_list(int n) {
  set_mode(mode_set_list);
  send_channel_message(CONTROLLER + channel, CC_BANK_SELECT_MSB, 0);
  send_channel_message(CONTROLLER + channel, CC_BANK_SELECT_LSB, (byte)(n & 0xff));
  send_channel_message(PROGRAM_CHANGE + channel, 0, 0);
}

// ================ helpers ================

void Kronos::dump_sysex(const char * const msg) {
  dump_hex(sysex.data(), sysex.size(), msg);
}
