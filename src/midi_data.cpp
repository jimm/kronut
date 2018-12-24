#include <string.h>
#include "midi_data.h"
#include "utils.h"

MIDIData::MIDIData(int init_type, byte *bytes, size_t len) {
  if (init_type == MD_INIT_MIDI) {
    midi_len = len;
    midi_to_internal_len();

    midi_bytes = (byte *)malloc(midi_len);
    memcpy(midi_bytes, bytes, midi_len);
    internal_bytes = (byte *)malloc(internal_len);
    midi_changed();
  }
  else {
    internal_len = len;
    internal_to_midi_len();

    internal_bytes = (byte *)malloc(internal_len);
    memcpy(internal_bytes, bytes, internal_len);
    midi_bytes = (byte *)malloc(midi_len);
    internal_changed();
  }
}

MIDIData::~MIDIData() {
  free(midi_bytes);
  free(internal_bytes);
}


// Tells this object that you've changed the MIDI data so it can recalculate
// the internal data.
void MIDIData::midi_changed() {
  midi_to_internal();
}

// Tells this object that you've changed the internal data so it can
// recalculate the MIDI data.
void MIDIData::internal_changed() {
  internal_to_midi();
}

void MIDIData::dump() {
  dump_hex(midi_bytes, midi_len, "MIDIData midi bytes");
  dump_hex(internal_bytes, internal_len, "MIDIData internal bytes");
}

void MIDIData::midi_to_internal() {
  size_t mlen = midi_len;
  byte *midi = midi_bytes;
  byte *internal = internal_bytes;

  while (mlen > 0) {
    int chunk_len = 8 > mlen ? mlen : 8;
    for (int i = 0; i < chunk_len-1; ++i) {
      internal[i] = midi[i+1] + ((midi[0] & (1 << i)) ? 0x80 : 0);
    }
    midi += chunk_len;
    mlen -= chunk_len;
    internal += chunk_len-1;
  }
}

void MIDIData::internal_to_midi() {
  size_t ilen = internal_len;
  byte *internal = internal_bytes;
  byte *midi = midi_bytes;

  while (ilen > 0) {
    int chunk_len = 7 > ilen ? ilen : 7;
    midi[0] = 0;
    for (int i = 0; i < chunk_len; ++i) {
      if (internal[i] & 0x80)
        midi[0] += (1 << i);
      midi[i+1] = internal[i] & 0x7f;
    }
    internal += chunk_len;
    ilen -= chunk_len;
    midi += chunk_len+1;
  }
}

// Set internal_len from midi_len. Formula is from Kronos docs.
void MIDIData::midi_to_internal_len() {
  internal_len = (midi_len/8)*7 + (midi_len%8 ? midi_len%8-1 : 0);
}

// Set midi_len from internal_len. Formula is from Kronos docs.
void MIDIData::internal_to_midi_len() {
  midi_len = internal_len + (internal_len+6)/7;
}
