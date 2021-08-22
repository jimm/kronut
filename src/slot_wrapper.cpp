#include <iostream>
#include <strstream>
#include <iomanip>
#include "slot_wrapper.h"

const char * const SLOT_PERF_TYPE_NAMES[3] = {
  "Combination",
  "Program",
  "Song"
};

const char * const SLOT_PERF_TYPE_NAMES_ABBREV[3] = {
  "Combi",
  "Prog",
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

// Returns a non-zero value if `str` is too long.
int SlotWrapper::set_name(string str) {
  return string_to_chars(slot.name, SLOT_NAME_LEN, str);
}

SlotPerformanceType SlotWrapper::performance_type() {
  return (SlotPerformanceType)(slot.performance_type & 0x03);
}

void SlotWrapper::set_performance_type(SlotPerformanceType val) {
  slot.performance_type = (slot.performance_type & 0xfc) + ((int)val & 0x03);
}

const char * const SlotWrapper::performance_type_name(bool abbreviated) {
  int index = (int)performance_type();
  if (index < 0 || index >= (sizeof(SLOT_PERF_TYPE_NAMES) / sizeof(const char * const))) {
    cerr << "error: illegal performance type value " << index << endl;
    exit(1);
  }
  return abbreviated ? SLOT_PERF_TYPE_NAMES_ABBREV[index] : SLOT_PERF_TYPE_NAMES[index];
}

void SlotWrapper::set_performance_bank(byte val) {
  slot.performance_bank = (slot.performance_bank & 0xe0) + (val & 0x1f);
}

byte SlotWrapper::performance_bank() {
  return slot.performance_bank & 0x1f;
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
      ostream << (char)('A' + bank - 0x11) << std::ends;
    return ostream.str();
  }
  if (bank == 0x10)
    return "g(d)";
  if (bank >= 0x07) {
    ostream << "g(" << (int)(bank - 0x07 + 1) << ')' << std::ends;
    return ostream.str();
  }
  if (bank == 0x06)
    return "GM";

  ostream << "INT-" << (char)('A' + (int)bank) << std::ends;
  return ostream.str();
}

byte SlotWrapper::performance_index() {
  return slot.performance_index;
}

void SlotWrapper::set_performance_index(byte val) {
  slot.performance_index = val;
}

string SlotWrapper::performance_name() {
  ostrstream ostream;
  int index_offset = 0;

  // The g* banks and the GM bank start with index 1, not 0, in the UI.
  if (performance_bank() >= 0x06 && performance_bank() <= 0x10)
    index_offset = 1;

  ostream << performance_type_name(true) << ' ' << performance_bank_name() << ' '
          << std::setfill('0') << std::setw(3) << (performance_index() + index_offset)
          << std::ends;
  return ostream.str();
}

// Parses str and sets performance type, bank, and index. Everything is case
// insensitive.
//
// Accepts any prefix of performance type names (combi, prog, song). Only
// the first letter is checked.
//
// Bank names must match "INT-[A-F]", "GM", "g\([1-9d]\)", or
// "USER-[A-G]{1,2}".
void SlotWrapper::set_performance_name(string str) {
  for (int i = 0; i < 3; ++i) {
    if (strncasecmp(str.c_str(), SLOT_PERF_TYPE_NAMES[i], 1) == 0) {
      set_performance_type((SlotPerformanceType)i);
      break;
    }
  }

  size_t index = str.find(' ');
  if (index == string::npos)
    return;

  str = str.substr(index + 1);
  if (strncasecmp(str.c_str(), "USER-", 5) == 0) {
    char ch = str.c_str()[5];
    if (ch >= 'a' && ch <= 'g')
      ch = ch - 'a' + 'A';
    byte val = 0x11 + ch - 'A';
    if (str.c_str()[6] == ch)
      val += 7;
    set_performance_bank(val);
  }
  else if (strncasecmp(str.c_str(), "g(d)", 4) == 0) {
    set_performance_bank(0x10);
  }
  else if (strncasecmp(str.c_str(), "g(", 2) == 0) {
    set_performance_bank(0x07 + (str.c_str()[2] - '1'));
  }
  else if (strncasecmp(str.c_str(), "GM", 2) == 0) {
    set_performance_bank(0x06);
  }
  else if (strncasecmp(str.c_str(), "INT-", 4) == 0) {
    char ch = str.c_str()[4];
    if (ch >= 'a' && ch <= 'f')
      ch = ch - 'a' + 'A';
    byte val = ch - 'A';
    set_performance_bank(val);
  }

  index = str.find(' ');
  if (index == string::npos)
    return;

  int perf_index_offset = 0;
  if (performance_bank() >= 0x06 && performance_bank() <= 0x10)
    perf_index_offset = -1;
  long val = strtol(str.substr(index + 1).c_str(), 0, 10);
  set_performance_index((int)val + perf_index_offset);
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

void SlotWrapper::set_color(SlotColor c) {
  slot.performance_type = (slot.performance_type & 0xc3) + ((c << 2) & 0x3c);
}

const char * const SlotWrapper::color_name() {
  return SLOT_COLOR_NAMES[color()];
}

void SlotWrapper::set_color_name(string str) {
  for (int i = 0; i < 16; ++i) {
    int len = str.size();
    if (len > strlen(SLOT_COLOR_NAMES[i]))
      len = strlen(SLOT_COLOR_NAMES[i]);
    if (strncasecmp(str.c_str(), SLOT_COLOR_NAMES[i], len) == 0) {
      set_color((SlotColor)i);
      return;
    }
  }
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
    slot.keyboard_track &= 0xef;
    slot.performance_type = (slot.performance_type & 0x3f) + (((int)f << 6) & 0xc0);
  }
}

void SlotWrapper::set_font_name(string str) {
  for (int i = 0; i < 5; ++i) {
    int len = str.size();
    if (len > strlen(SLOT_FONT_NAMES[i]))
      len = strlen(SLOT_FONT_NAMES[i]);
    if (strncasecmp(str.c_str(), SLOT_FONT_NAMES[i], len) == 0) {
      set_font((SlotFont)i);
      return;
    }
    if (strncasecmp(str.c_str(), SLOT_FONT_SHORT_NAMES[i], strlen(SLOT_FONT_SHORT_NAMES[i])) == 0) {
      set_font((SlotFont)i);
      return;
    }
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

// Returns a non-zero value if `str` is too long.
int SlotWrapper::set_comments(string str) {
  return string_to_chars(slot.comments, SLOT_COMMENTS_LEN, str);
}
