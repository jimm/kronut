#ifndef SLOT_WRAPPER_H
#define SLOT_WRAPPER_H

#include <string>
#include "slot.h"

using namespace std;

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

class SlotWrapper {
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

  SlotFont font();
  void set_font(SlotFont f);

  int xpose();
  void set_xpose(int xpose);

  string comments();
  void set_comments(string str);

private:
  string chars_to_string(byte *, int len);
  void string_to_chars(byte *, int len, string str);
};

#endif /* SLOT_WRAPPER_H */
