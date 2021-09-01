#include <iostream>
#include <iomanip>
#include <strstream>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/errno.h>
#include "editor.h"
#include "set_list_wrapper.h"
#include "slot_wrapper.h"
#include "set_list_file.h"

Editor::Editor(int format)
  : _file_format(format)
{
  switch (_file_format) {
  case EDITOR_FORMAT_ORG_MODE:
    _file = new OrgModeSetListFile();
    break;
  case EDITOR_FORMAT_MARKDOWN:
    _file = new MarkdownSetListFile();
    break;
  case EDITOR_FORMAT_HEXDUMP:
    _file = nullptr;
    break;
  }
}

Editor::~Editor() {
  if (_file != nullptr) delete _file;
}

int Editor::load_set_list_from_file(const char * const path) {
  SetListWrapper slw(_set_list);
  int slot_number = 0;
  bool collect_comments = false;
  string name;
  string comments;

  if (!_file->open(path, "r")) {
    cerr << "error: can't open \"" << path << "\" for reading: " << strerror(errno) << endl;
    return errno;
  }

  init_set_list();

  while (_file->getline()) {
    if (_file->is_header(1)) {
      // Set List name
      name = _file->header_text(1);
      if (slw.set_name(trimmed(name)) != 0)
        cerr << "warning: set list name \"" << name << "\" is too long and will be truncated" << endl;

      _file->skip_blank_lines();
      load_set_list_settings_from_file(slw);
    }
    else if (_file->is_header(2)) {
      // Slot name, beginning of comments
      name = _file->header_text(2);
      comments = "";
      collect_comments = true;
    }
    else if (_file->is_table_start()) {
      collect_comments = false;

      Slot &slot = _set_list.slots[slot_number];
      SlotWrapper sw(slot);
      if (sw.set_name(trimmed(name)) != 0)
        cerr << "warning: slot " << setw(3) << setfill('0') << slot_number
             << " named \"" << name << "\" is too long and will be truncated"
             << endl;

      string trimmed_comments = trimmed(comments);
      if (sw.set_comments(trimmed_comments) != 0)
        cerr << "warning: slot " << setw(3) << setfill('0') << slot_number
             << " named \"" << name
             << "\" comments \""
             << trimmed_comments.substr(0, 20) << "...\" are too long and will be truncated"
             << endl;

      load_set_list_slot_settings_from_file(sw);

      ++slot_number;
      if (slot_number >= 128)
        break;
    }
    else {
      if (collect_comments) {
        comments += _file->line();
        comments += '\n';
      }
    }
  }
  _file->close();
  return 0;
}

void Editor::load_set_list_settings_from_file(SetListWrapper &sw) {
  memset(sw.set_list.reserved, 0, 3);

  _file->skip_table_headers();
  while (!_file->is_table_separator()) {
    string setting_name = _file->table_col1();
    string value = _file->table_col2();

    if (setting_name == "Slots/Page")
      sw.set_slots_per_page(atoi(value.c_str()));
    else if (setting_name == "EQ Bypass")
      _set_list.eq_bypass = atoi(value.c_str());
    else if (setting_name == "Band Levels") {
      char *p = (char *)(const char *)value.c_str();
      for (int i = 0; i < 9; ++i) {
        char *endptr;
        long val = strtol(p, &endptr, 10);
        _set_list.band_levels[i] = (byte)val;
        p = endptr + 1;
      }
    }
    else if (setting_name == "Surface Mode")
      _set_list.control_surface_mode = atoi(value.c_str());
    else if (setting_name == "Surface Asgn")
      sw.set_control_surface_assign_from(value);
    _file->getline();
  }

  _file->getline();
}

// Loads set list slot settings and returns original set list slot index.
void Editor::load_set_list_slot_settings_from_file(SlotWrapper &sw) {
  _file->skip_table_headers();
  while (!_file->is_table_separator()) {
    string setting_name = _file->table_col1();
    string value = _file->table_col2();
    if (setting_name == "Performance")
      sw.set_performance_name(value);
    else if (setting_name == "Color")
      sw.set_color_name(value);
    else if (setting_name == "Font")
      sw.set_font_name(value);
    else if (setting_name == "Transpose")
      sw.set_xpose(atoi(value.c_str()));
    else if (setting_name == "Volume")
      sw.set_volume(atoi(value.c_str()));
    else if (setting_name == "Hold Time")
      sw.set_hold_time(atoi(value.c_str()));
    else if (setting_name == "Kbd Track")
      sw.set_keyboard_track(atoi(value.c_str()));
    _file->getline();
  }
}

int Editor::save_set_list_to_file(const char * const path, bool skip_empty_slots) {
  if (_file_format == EDITOR_FORMAT_HEXDUMP)
    return hexdump(path);

  char buf[BUFSIZ];
  SetListWrapper slw(_set_list);

  if (!_file->open(path, "w")) {
    cerr << "error: can't open \"" << path << "\" for writing: " << strerror(errno) << endl;
    return errno;
  }

  _file->header(1, slw.name());
  save_set_list_settings_to_file(slw);
  _file->puts("");

  for (int i = 0; i < 128; ++i) {
    Slot &slot = _set_list.slots[i];
    SlotWrapper sw(slot);

    if (skip_empty_slots && sw.is_empty())
      continue;

    _file->header(2, sw.name());
    if (sw.comments().size() > 0)
      _file->text(trimmed(sw.comments()));
    save_set_list_slot_settings_to_file(sw);
    _file->puts("");
  }

  _file->close();

  return 0;
}

void Editor::save_set_list_settings_to_file(SetListWrapper &slw) {
  ostrstream ostr;

  _file->table_headers("Setting", "Value");
  _file->table_row("Slots/Page", slw.slots_per_page());
  _file->table_row("EQ Bypass", _set_list.eq_bypass);
  for (int i = 0 ; i < 9; ++i) {
    if (i > 0) ostr << ',';
    ostr << (int)_set_list.band_levels[i];
  }
  _file->table_row("Band Levels", ostr.str());
  _file->table_row("Surface Mode", _set_list.control_surface_mode);
  _file->table_row("Surface Asgn", slw.control_surface_assign_from_name());
  _file->table_end();
}

void Editor::save_set_list_slot_settings_to_file(SlotWrapper &sw) {
  _file->table_headers("Setting", "Value");
  _file->table_row("Performance", sw.performance_name().c_str());
  _file->table_row("Color", sw.color_name());
  _file->table_row("Font", sw.font_name());
  _file->table_row("Transpose", sw.xpose());
  _file->table_row("Volume", sw.volume());
  _file->table_row("Hold Time", sw.hold_time());
  _file->table_row("Kbd Track", sw.keyboard_track());
  _file->table_end();
}

int Editor::hexdump(const char * const path) {
  ofstream out;

  out.open(path, std::ofstream::out);
  if (out.fail()) {
    cerr << "error opening " << path << " for output" << endl;
    exit(1);
  }

  byte *bytes = (byte *)&_set_list;
  size_t size = sizeof(_set_list);
  size_t offset = 0;

  // TODO this duplicates dump_hex code in utils.cpp
  while (size > 0) {
    int chunk_len = 8 > size ? size : 8;
    out << setw(8) << setfill('0') << hex << offset << ' ';
    out << "  ";
    for (int i = 0; i < chunk_len; ++i)
      out << ' ' << setw(2) << setfill('0') << hex << (int)bytes[i];
    for (int i = chunk_len; i < 8; ++i)
      out << "   ";
    out << ' ';
    for (int i = 0; i < chunk_len; ++i)
      out << (char)((bytes[i] >= 32 && bytes[i] < 127) ? bytes[i] : '.');
    out << endl;
    bytes += chunk_len;
    size -= chunk_len;
    offset += chunk_len;
  }

  out.close();

  return 0;
}

string Editor::trimmed(string s) {
  char buf[BUFSIZ], *p;

  strncpy(buf, s.c_str(), BUFSIZ-1);
  buf[BUFSIZ-1] = '\0';
  for (p = buf; *p && isspace(*p); ++p) ;
  for (char *q = p + strlen(p) -1; q >= p && isspace(*q); --q)
    *q = 0;
  return string(p);
}

void Editor::init_set_list() {
  SetListWrapper slw(_set_list);

  memset((void *)&_set_list, 0, sizeof(SetList));
  slw.set_name("Empty Set List");
  slw.set_slots_per_page(16);
  _set_list.eq_bypass = 1;
  _set_list.control_surface_mode = 5;
  for (int i = 0; i < 128; ++i) {
    Slot &slot = _set_list.slots[i];
    SlotWrapper sw(slot);
    sw.set_performance_type(pt_program);
    sw.set_volume(127);
    sw.set_hold_time(6);
  }
}
