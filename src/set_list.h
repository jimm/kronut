#pragma once

#include "types.h"
#include "slot.h"

// If a set list is uninitialized then it's first byte (name[0]) is this
// value.
#define UNINITIALIZED_SET_LIST 0xff
#define SET_LIST_NAME_LEN 24
#define UNDEFINED_SET_LIST_NUM (-1)

using namespace std;

struct SetList {
  byte name[SET_LIST_NAME_LEN];
  Slot slots[128];
  byte eq_bypass;
  byte band_levels[9];
  byte control_surface_mode;
  byte control_surface_assign_from; // 0 == Slot, 1 == Set List
  byte slots_per_page;          // 0 == 16, 1 == 8, 2 == 4
  byte reserved[3];

  SetList() { name[0] = UNINITIALIZED_SET_LIST; }

  bool is_initialized() { return name[0] != UNINITIALIZED_SET_LIST; }
};
