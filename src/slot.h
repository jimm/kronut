#ifndef SLOT_H
#define SLOT_H

// Name and comments lengths are for "internal bytes", not MIDI byte length
#define SLOT_NAME_LEN 24
#define SLOT_COMMENTS_LEN 512

typedef unsigned char byte;

struct Slot {
  byte name[SLOT_NAME_LEN];
  byte performance_type;
  byte performance_bank;
  byte performance_index;
  byte hold_time;
  byte volume;
  byte keyboard_track;
  byte comments[SLOT_COMMENTS_LEN];
};

#endif /* SLOT_H */
