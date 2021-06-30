#include "slot_wrapper.h"

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

string SlotWrapper::name() {
  return chars_to_string(slot.name, SLOT_NAME_LEN);
}

void SlotWrapper::set_name(string str) {
  string_to_chars(slot.name, SLOT_NAME_LEN, str);
}

byte SlotWrapper::performance_type() {
  return slot.performance_type & 0x03;
}

void SlotWrapper::set_performance_type(byte val) {
  slot.performance_type = (slot.performance_type & 0xfc) + (val & 0x03);
}

byte SlotWrapper::performance_bank() {
  return slot.performance_bank & 0x1f;
}

void SlotWrapper::set_performance_bank(byte val) {
  slot.performance_bank = (slot.performance_bank & 0xe0) + (val & 0x1f);
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
