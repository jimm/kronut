#pragma once

#include <string>
#include "types.h"
#include "slot.h"
#include "struct_wrapper.h"

using namespace std;

// Color takes bits 2-5 of performance_type
// Font takes bits

enum SlotPerformanceType {
  pt_combination = 0,
  pt_program,
  pt_song
};

enum SlotColor {
  color_default = 0,
  color_charcoal,
  color_brick,
  color_burgundy,
  color_ivy,
  color_olive,
  color_gold,
  color_cacao,
  color_indigo,
  color_navy,
  color_rose,
  color_lavender,
  color_azure,
  color_denim,
  color_silver,
  color_slate
};

enum SlotFont {
  font_s = 0,
  font_xs,
  font_m,
  font_l,
  font_xl
};

extern const char * const SLOT_PERF_TYPE_NAMES[3];
extern const char * const SLOT_COLOR_NAMES[16];
extern const char * const SLOT_FONT_NAMES[5];
extern const char * const SLOT_FONT_SHORT_NAMES[5];

class SlotWrapper : public StructWrapper {
public:
  Slot &slot;

  SlotWrapper(Slot &s) : slot(s) {}

  bool is_empty();

  string name();
  int set_name(string str);

  SlotPerformanceType performance_type();
  void set_performance_type(SlotPerformanceType val);
  const char * const performance_type_name(bool abbreviated);

  byte performance_bank();
  string performance_bank_name();
  void set_performance_bank(byte val);

  byte performance_index();
  void set_performance_index(byte val);

  string performance_name();
  // Parses str and sets performance type, bank, and index.
  void set_performance_name(string str);

  byte hold_time();
  void set_hold_time(byte val);

  byte volume();
  void set_volume(byte val);

  byte keyboard_track();
  void set_keyboard_track(byte val);

  SlotColor color();
  const char * const color_name();
  void set_color(SlotColor c);
  void set_color_name(string str);

  SlotFont font();
  void set_font(SlotFont f);
  const char * const font_name();
  const char * const font_short_name();
  void set_font_name(string str);

  int xpose();
  void set_xpose(int xpose);

  string comments();
  int set_comments(string str);
};
