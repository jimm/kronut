#ifndef MIDI_DATA_H
#define MIDI_DATA_H

#include <stdlib.h>

typedef unsigned char byte;

#define MD_INIT_MIDI 0
#define MD_INIT_INTERNAL 1

// Auto-converts between MIDI bytes and internal bytes. Keeps them in sync.
class MIDIData {
public:
  byte *midi_bytes;
  byte *internal_bytes;
  size_t midi_len;
  size_t internal_len;

  MIDIData(int init_type, byte *bytes, size_t len);
  virtual ~MIDIData();

  // Tells this object that you've changed the MIDI data so it can
  // recalculate the internal data.
  virtual void midi_changed();

  // Tells this object that you've changed the internal data so it can
  // recalculate the MIDI data.
  virtual void internal_changed();

  void dump();

private:

  void midi_to_internal();
  void internal_to_midi();

  void midi_to_internal_len();
  void internal_to_midi_len();
};

#endif /* MIDI_DATA_H */