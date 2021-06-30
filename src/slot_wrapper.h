#ifndef SLOT_WRAPPER_H
#define SLOT_WRAPPER_H

#include <string>
#include "slot.h"
#include "struct_wrapper.h"

using namespace std;

typedef unsigned char byte;

// Color takes bits 2-5 of performance_type
// Font takes bits

enum SlotColor {
  color_default,
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
  font_s,
  font_xs,
  font_m,
  font_l,
  font_xl
};

extern const char * const SLOT_COLOR_NAMES[16];
extern const char * const SLOT_FONT_NAMES[5];

class SlotWrapper : public StructWrapper {
public:
  Slot &slot;

  SlotWrapper(Slot &s) : slot(s) {}

  string name();
  void set_name(string str);

  byte performance_type();
  void set_performance_type(byte val);

  byte performance_bank();
  void set_performance_bank(byte val);

  byte performance_index();
  void set_performance_index(byte val);

  byte hold_time();
  void set_hold_time(byte val);

  byte volume();
  void set_volume(byte val);

  byte keyboard_track();
  void set_keyboard_track(byte val);

  SlotColor color();
  void set_color(SlotColor c);
  const char * const color_name();

  SlotFont font();
  void set_font(SlotFont f);
  const char * const font_name();

  int xpose();
  void set_xpose(int xpose);

  string comments();
  void set_comments(string str);
};

#endif /* SLOT_WRAPPER_H */
