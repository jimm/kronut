#include <strstream>
#include <iomanip>
#include "slot_wrapper.h"

const char * const SLOT_PERF_TYPE_NAMES[3] = {
  "Program",
  "Combination",
  "Song"
};

const char * const SLOT_COLOR_NAMES[16] = {
  "Default",
  "Charcoal",
  "Brick",
  "Burgundy",
  "Ivy",
  "Olive",
  "Gold",
  "Cacao",
  "Indigo",
  "Navy",
  "Rose",
  "Lavender",
  "Azure",
  "Denim",
  "Silver",
  "Slate"
};

const char * const SLOT_FONT_NAMES[5] = {
  "Small", "Extra Small", "Medium", "Large", "Extra Large"
};

const char * const SLOT_FONT_SHORT_NAMES[5] = {
  "S", "XS", "M", "L", "XL"
};

string SlotWrapper::name() {
  return chars_to_string(slot.name, SLOT_NAME_LEN);
}

void SlotWrapper::set_name(string str) {
  string_to_chars(slot.name, SLOT_NAME_LEN, str);
}

SlotPerformanceType SlotWrapper::performance_type() {
  return (SlotPerformanceType)(slot.performance_type & 0x03);
}

void SlotWrapper::set_performance_type(SlotPerformanceType val) {
  slot.performance_type = (slot.performance_type & 0xfc) + ((int)val & 0x03);
}

const char * const SlotWrapper::performance_type_name() {
  return SLOT_PERF_TYPE_NAMES[(int)performance_type()];
}

byte SlotWrapper::performance_bank() {
  return slot.performance_bank & 0x1f;
}

void SlotWrapper::set_performance_bank(byte val) {
  slot.performance_bank = (slot.performance_bank & 0xe0) + (val & 0x1f);
}

string SlotWrapper::performance_bank_name() {
  ostrstream ostream;

  byte bank = performance_bank();
  if (bank >= 0x11) {
    ostream << "USER-";
    if (bank >= 0x18) {
      ostream << (char)('A' + bank - 0x18);
      ostream << (char)('A' + bank - 0x18);
    }
    else
      ostream << (char)('A' + bank - 0x11);
    return ostream.str();
  }
  if (bank == 0x10)
    return "g(d)";
  if (bank >= 0x07) {
    ostream << "g(" << (int)(bank - 0x07 + 1) << ')';
    return ostream.str();
  }
  if (bank == 0x06)
    return "GM";

  ostream << "INT-" << (char)('A' + bank);
  return ostream.str();
}

string SlotWrapper::performance_name() {
  ostrstream ostream;
  ostream << performance_type_name() << ' ' << performance_bank_name() << ' '
          << std::setfill('0') << std::setw(3) << performance_index();
  return ostream.str();
}

byte SlotWrapper::performance_index() {
  return slot.performance_index;
}

void SlotWrapper::set_performance_index(byte val) {
  slot.performance_index = val;
}

byte SlotWrapper::hold_time() {
  return slot.hold_time;
}

void SlotWrapper::set_hold_time(byte val) {
  slot.hold_time = val;
}

byte SlotWrapper::volume() {
  return slot.volume;
}

void SlotWrapper::set_volume(byte val) {
  slot.volume = val;
}

byte SlotWrapper::keyboard_track() {
  return slot.keyboard_track & 0x0f;
}

void SlotWrapper::set_keyboard_track(byte val) {
  slot.keyboard_track = (slot.keyboard_track & 0xf0) + (val & 0x0f);
}

SlotColor SlotWrapper::color() {
  byte val = (slot.performance_type & 0x3c) >> 2;
  return (SlotColor)(val);
}

const char * const SlotWrapper::color_name() {
  return SLOT_COLOR_NAMES[color()];
}

void SlotWrapper::set_color(SlotColor c) {
  slot.performance_type = (slot.performance_type & 0xc3) + ((c << 2) & 0x3c);
}

SlotFont SlotWrapper::font() {
  if (slot.keyboard_track & 0x10)
    return font_xl;
  return (SlotFont)((slot.performance_type >> 6) & 0x03);
}

const char * const SlotWrapper::font_name() {
  return SLOT_FONT_NAMES[font()];
}

const char * const SlotWrapper::font_short_name() {
  return SLOT_FONT_SHORT_NAMES[font()];
}

void SlotWrapper::set_font(SlotFont f) {
  if (f == font_xl) {
    slot.keyboard_track |= 0x10;
    slot.performance_type &= 0x3f;
  }
  else {
    slot.keyboard_track &= 0x7f;
    slot.performance_type = (slot.performance_type & 0x3f) + ((f << 6) & 0xc0);
  }
}

int SlotWrapper::xpose() {
  int xpose = ((slot.performance_bank >> 5) << 3) + (slot.keyboard_track >> 5);
  if (xpose > 24)
    xpose -= 64;
  return xpose;
}

void SlotWrapper::set_xpose(int xpose) {
  if (xpose < 0)
    xpose += 64;
  slot.keyboard_track = (slot.keyboard_track & 0x1f) + ((xpose & 0x07) << 5);
  slot.performance_bank = (slot.performance_bank & 0x1f) + ((xpose & 0xf8) << 2);
}

string SlotWrapper::comments() {
  return chars_to_string(slot.comments, SLOT_COMMENTS_LEN);
}

void SlotWrapper::set_comments(string str) {
  string_to_chars(slot.comments, SLOT_COMMENTS_LEN, str);
}
