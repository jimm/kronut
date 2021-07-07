#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <unistd.h>
#include "editor.h"
#include "set_list_wrapper.h"
#include "slot_wrapper.h"
#include "set_list_file.h"

Editor::Editor(int format)
{
  if (format == EDITOR_FORMAT_ORG_MODE)
    _file = new OrgModeSetListFile();
  else
    _file = new MarkdownSetListFile();
}

// If path is nullptr, reads from stdin.
void Editor::load_set_list_from_file(char *path) {
  SetListWrapper slw(_set_list);
  int slot_number = 0;
  bool collect_comments = false;
  string name;
  string comments;

  // in case we get fewer than 128 slots
  memset((void *)&_set_list, 0, sizeof(SetList));

  if (_file->open(path, "r") != 0) {
    fprintf(stderr, "error: can't open \"%s\" for reading: %s\n",
            path, strerror(errno));
    return;
  }

  while (_file->gets()) {
    if (_file->is_header(1)) {
      // Set List name
      name = _file->header_text(1);
      slw.set_name(name);

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
      sw.set_name(name);
      sw.set_comments(trimmed(comments));
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
      _set_list.control_surface_assign_from = atoi(value.c_str());
    _file->gets();
  }

  _file->gets();
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
    _file->gets();
  }

  _file->gets();
}

// If path is nullptr, writes to stdout.
void Editor::save_set_list_to_file(char *path) {
  char buf[BUFSIZ];
  SetListWrapper slw(_set_list);

  if (_file->open(path, "w") == nullptr) {
    fprintf(stderr, "error: can't open \"%s\" for writing: %s\n",
            path, strerror(errno));
    return;
  }

  _file->header(1, slw.name());
  save_set_list_settings_to_file(slw);
  _file->puts("");

  for (int i = 0; i < 128; ++i) {
    Slot &slot = _set_list.slots[i];
    SlotWrapper sw(slot);

    _file->header(2, sw.name());
    if (sw.comments().size() > 0)
      _file->text(trimmed(sw.comments()));
    save_set_list_slot_settings_to_file(sw);
    _file->puts("");
  }

  _file->close();
}

void Editor::save_set_list_settings_to_file(SetListWrapper &slw) {
  char buf[BUFSIZ];

  _file->table_headers("Setting", "Value");
  _file->table_row("Slots/Page", slw.slots_per_page());
  _file->table_row("EQ Bypass", _set_list.eq_bypass);
  sprintf(buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d", _set_list.band_levels[0],
          _set_list.band_levels[1], _set_list.band_levels[2], _set_list.band_levels[3],
          _set_list.band_levels[4], _set_list.band_levels[5], _set_list.band_levels[6],
          _set_list.band_levels[7], _set_list.band_levels[8]);
  _file->table_row("Band Levels", buf);
  _file->table_row("Surface Mode", _set_list.control_surface_mode);
  _file->table_row("Surface Asgn", _set_list.control_surface_assign_from);
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

string Editor::trimmed(string s) {
  char buf[BUFSIZ], *p;

  strncpy(buf, s.c_str(), BUFSIZ-1);
  buf[BUFSIZ-1] = '\0';
  for (p = buf; *p && isspace(*p); ++p) ;
  for (char *q = p + strlen(p) -1; q >= p && isspace(*q); --q)
    *q = 0;
  return string(p);
}
