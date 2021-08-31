#ifndef SLOT_H
#define SLOT_H

// Name and comments lengths are for "internal bytes", not MIDI byte length
#define SLOT_NAME_LEN 24
#define SLOT_COMMENTS_LEN 512

typedef unsigned char byte;

/*
 * performance type
 *
 *  7 6 5 4 3 2 1 0
 *  |_| |_____| |_|
 * font  color  type
 * low 2
 * bits
 *
 *
 * performance bank
 *
 *  7 6 5 4 3 2 1 0
 *  |___| |_______|
 * xpose   perf bank
 * hi bits
 *
 *   perf bank:
 *     INT A - INT-F     = 0x00-0x05
 *     GM                = 0x06
 *     g(1) - g(9)       = 0x07-0x0f
 *     g(d)              = 0x10
 *     USER-A - USER-G   = 0x11-0x17
 *     USER-AA - USER-GG = 0x18-1E
 *
 *
 * performance index
 *  (all bits used (?))
 *
 *
 * hold time
 *  (all bits used (?))
 *
 *
 * volume
 *  (all bits used)
 *
 *
 * keyboard track
 *
 *  7 6 5 4 3 2 1 0
 *  |___| | |_____|
 *  xpose |  kbd track
 *  low   |
 *       font high bit
 */

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
