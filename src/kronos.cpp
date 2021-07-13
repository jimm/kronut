#include <iostream>
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

#define READ_SYSEX_TIMEOUT_SECS 5
#define TIMEOUT_ERROR_REPLY 100

#define is_realtime(b) ((b) >= CLOCK)

enum SysexState {
  waiting,
  receiving,
  received,
  error
};

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

Kronos *Kronos_instance() {
  return kronos_instance;
}

// ================ allocation ================

// chan must be 0-15
Kronos::Kronos(byte chan, int input_device_num, int output_device_num)
  : channel(chan)
{
  if (input_device_num >= 0) {  // negative means we're testing
    PmError err = Pm_OpenInput(&input, input_device_num, 0, MIDI_BUFSIZ, 0, 0);
    if (err != 0) {
      fprintf(stderr, "error opening PortMidi input stream: %s\n", Pm_GetErrorText(err));
      exit(1);
    }
  }

  if (output_device_num >= 0) {
    PmError err = Pm_OpenOutput(&output, output_device_num, 0, MIDI_BUFSIZ, 0, 0, 0);
    if (err != 0) {
      fprintf(stderr, "error opening PortMidi output stream: %s\n", Pm_GetErrorText(err));
      exit(1);
    }
  }

  kronos_instance = this;
}

Kronos::~Kronos() {
  if (kronos_instance == this)
    kronos_instance = nullptr;
}

// ================ sysex I/O ================

void Kronos::send_sysex(const byte * const sysex_bytes) {
  PmError err = Pm_WriteSysEx(output, 0, (unsigned char *)sysex_bytes);
  if (err != 0) {
    fprintf(stderr, "error writing sysex: %s\n", Pm_GetErrorText(err));
    exit(1);
  }
}

// Wait for next System Exclusive message to be read into `sysex`. We first
// receive and dump everything as quickly as we can into a ByteData. Then we
// post-process it, removing realtime bytes and any bytes before or after
// the sysex.
void Kronos::read_sysex() {
  PmEvent buf[SYSEX_BUF_EVENTS];
  ByteData raw_bytes;
  SysexState state;
  time_t start = time(0);

  state = waiting;
  while (state != received && state != error) {
    if (Pm_Poll(input) == TRUE) {
      int n = Pm_Read(input, buf, SYSEX_BUF_EVENTS);
      for (int i = 0; i < n; ++i) {
        PmMessage msg = buf[i].message;
        for (int j = 0; j < 4; ++j) {
          byte b = msg & 0xff;
          msg >>= 8;
          raw_bytes.append(b);
          if (b == EOX)
            state = received;
        }
      }
    }
    else if ((time(0) - start) >= READ_SYSEX_TIMEOUT_SECS) {
      fprintf(stderr, "timeout waiting for sysex\n");
      exit(1);
    }
  }

  // Filter out realtime bytes and bytes before and after sysex bytes.
  sysex.clear();
  state = waiting;
  for (size_t i = 0; i < raw_bytes.size() && state != received && state != error; i += 4) {
    for (int j = 0; j < 4; ++j) {
      byte b = raw_bytes[i + j];
      switch (b) {
      case SYSEX:
        state = receiving;
        sysex.append(b);
        break;
      case EOX:
        sysex.append(b);
        state = received;
        break;
      default:
        if (state == receiving && !is_realtime(b))
          sysex.append(b);
        break;
      }
    }
  }
}

void Kronos::get(const byte * const request_sysex, const char * const func_name) {
  send_sysex(request_sysex);
  read_sysex();
  if (error_reply_seen()) {
    fprintf(stderr, "Kronos::%s received an error response: %s\n", func_name, error_reply_message());
    exit(1);
  }
}

void Kronos::send_channel_message(byte status, byte data1, byte data2) {
  PmError err = Pm_WriteShort(output, 0, Pm_Message(status, data1, data2));
  if (err != 0) {
    fprintf(stderr, "error writing channel message: %s\n", Pm_GetErrorText(err));
    exit(1);
  }
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
  get(request_sysex, "read_current_string");

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
  get(request_sysex, "read_current_set_list");

  int start = 7;
  int end = start;
  while (sysex[end] != EOX) ++end;

  MIDIData midi_data(MD_INIT_MIDI, &sysex.data()[start], end - start);
  memcpy((void *)&set_list, (void *)midi_data.internal_bytes, midi_data.internal_len);
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

  get(request_sysex, "write_current_string");
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

  get(request_sysex, "write_current_set_list");

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

  get(request_sysex, "save_current_set_list");
}

// ================ mode and movement commands ================

KronosMode Kronos::mode() {
  const byte request_sysex[] = {
    SYSEX_HEADER,
    FUNC_CODE_MODE_REQ, EOX
  };
  get(request_sysex, "mode");
  return (KronosMode)(sysex[5] & 0x0f);
}

void Kronos::set_mode(KronosMode mode) {
  const byte request_sysex[] = {
    SYSEX_HEADER,
    FUNC_CODE_MODE_CHANGE, (byte)mode, EOX
  };
  get(request_sysex, "set_mode");
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
