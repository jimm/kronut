#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <unistd.h>
#include "editor.h"
#include "set_list_wrapper.h"
#include "slot_wrapper.h"
#include "edit_file.h"

#define EDITOR_NAME 0
#define EDITOR_COMMENTS 1

Editor::Editor(Kronos *k)
  : kronos(k), name(""), comments("")
{
  char *env_value = getenv("KRONUT_FILE_MODE");
  if (env_value != nullptr && (env_value[0] == 'o' || env_value[0] == 'O'))
    file = new OrgModeEditFile();
  else
    file = new MarkdownEditFile();
}

int Editor::edit_current_slot(bool read_from_kronos) {
  if (read_from_kronos)
    read_slot();

  save_slot_to_file();
  int status = edit_file();
  if (status == 0) {
    load_slot_from_file();
    file->rm();

    if (name_too_long() || comments_too_long())
      return EDITOR_TOO_LONG;

    write_slot();
    return EDITOR_OK;
  }
  else
    return EDITOR_ERROR;
}

int Editor::edit_current_set_list(bool read_from_kronos) {
  if (read_from_kronos)
    kronos->read_current_set_list(set_list);

  save_set_list_to_file();
  int status = edit_file();
  if (status == 0) {
    load_set_list_from_file();
    file->rm();

    printf("Sending set list to Kronos. This will take a while...\n");
    kronos->write_current_set_list(set_list);
    printf("Done. Remember, the set list has not been saved.\n");
    return EDITOR_OK;
  }
  else
    return EDITOR_ERROR;
}

void Editor::print_current_slot() {
  read_slot();
  puts(name.c_str());
  puts("");
  puts(comments.c_str());
}

void Editor::print_set_list_slot_names() {
  kronos->read_current_set_list(set_list);
  SetListWrapper slw(set_list);
  printf("Set List: %s\n", slw.name().c_str());

  for (int i = 0; i < 128; ++i) {
    Slot &slot = set_list.slots[i];
    SlotWrapper sw(slot);
    printf("%3d\t%s\n", i + 1, sw.name().c_str());
  }
}

void Editor::print_set_list_slot_values() {
  kronos->read_current_set_list(set_list);
  SetListWrapper slw(set_list);

  printf("Set List: %s\n", slw.name().c_str());

  for (int i = 0; i < 128; ++i) {
    Slot &slot = set_list.slots[i];
    SlotWrapper sw(slot);

    printf("%3d\t%s\n", i + 1, sw.name().c_str());
    printf("\t{t: %02x, b: %02x, i: %02x, ht: %02x, v: %02x, trk: %02x}\n",
           slot.performance_type, slot.performance_bank, slot.performance_index,
           slot.hold_time, slot.volume, slot.keyboard_track);

    printf("\tcolor: %s, font: %s, xpose: %d\n", sw.color_name(), sw.font_name(), sw.xpose());
  }
}

void Editor::read_maybe_dump(bool dump) {
  KString *kstr;

  kstr = kronos->read_current_slot_name();
  name = kstr->str();
  if (dump)
    kronos->dump_sysex("slot name");
  delete kstr;

  kstr = kronos->read_current_slot_comments();
  comments = kstr->str();
  if (dump)
    kronos->dump_sysex("slot comments");
  delete kstr;
}

void Editor::save_slot_to_file() {
  file->open("w");
  file->header(1, "Slot Name");
  file->text(name);
  file->header(1, "Comments");
  file->text(comments);
  file->close();
}

void Editor::save_set_list_to_file() {
  char buf[BUFSIZ];
  SetListWrapper slw(set_list);

  file->open("w");
  file->header(1, slw.name());

  for (int i = 0; i < 128; ++i) {
    Slot &slot = set_list.slots[i];
    SlotWrapper sw(slot);

    file->header(2, sw.name());
    if (sw.comments().size() > 0)
      file->text(trimmed(sw.comments()));
    save_set_list_settings_to_file(sw, i);
    file->puts("");
  }

  file->close();
}

void Editor::save_set_list_settings_to_file(SlotWrapper &sw, int orig_slot) {
  file->table_headers("Setting", "Value");
  file->table_row("Performance", sw.performance_name().c_str());
  file->table_row("Color", sw.color_name());
  file->table_row("Font", sw.font_name());
  file->table_row("Transpose", sw.xpose());
  file->table_row("Volume", sw.volume());
  file->table_row("Hold Time", sw.hold_time());
  file->table_row("Kbd Track", sw.keyboard_track());
  file->table_separator();
  file->table_row("Orig. Slot", orig_slot);
  file->table_end();
}

int Editor::edit_file() {
  char buf[BUFSIZ];
  char *editor = getenv("VISUAL");
  char *options = getenv("KRONUT_VISUAL_OPTIONS");

  if (editor == 0) {
    editor = getenv("EDITOR");
    options = getenv("KRONUT_EDITOR_OPTIONS");
  }
  if (editor == 0) {
    editor = (char *)"vi";
    options = getenv("KRONUT_VI_OPTIONS");
  }
  if (options == 0)
    options = (char *)"";
  sprintf(buf, "%s %s %s 2>&1", editor, options, file->path().c_str());
  return system(buf);
}

// Loads name and comment from tempfile.
void Editor::load_slot_from_file() {
  int which = -1;
  string buf;

  file->open("r");
  while (file->gets() != 0) {
    if (file->is_header(1) && file->header_text(1) == "Slot Name") {
      which = EDITOR_NAME;
      buf = "";
    }
    else if (file->is_header(1) && file->header_text(1) == "Comments") {
      which = EDITOR_COMMENTS;
      name = trimmed(buf);
      buf = "";
    }
    else if (which != -1)
      buf += file->line();
  }
  comments = trimmed(buf);
  file->close();
}

void Editor::load_set_list_from_file() {
  char line[BUFSIZ];
  SetList new_set_list;
  SetListWrapper slw(new_set_list);
  long len;
  int slot_number = 0;
  bool collect_comments = false;

  memcpy((void *)&new_set_list, (void *)&set_list, sizeof(SetList));

  file->open("r");
  while (file->gets() != 0) {
    if (file->is_header(1)) {
      // Set List name
      name = file->header_text(1);
      slw.set_name(name);
    }
    else if (file->is_header(2)) {
      // Slot name, beginning of comments
      name = file->header_text(2);
      comments = "";
      collect_comments = true;
    }
    else if (file->is_header(3)) {
      collect_comments = false;
    }
    else if (file->is_table_start()) {
      Slot &slot = new_set_list.slots[slot_number];
      SlotWrapper sw(slot);
      int orig_slot_number = load_set_list_slot_settings_from_file(sw);

      // copy original slot into this slot position
      memcpy((void *)&slot, (void *)&set_list.slots[orig_slot_number], sizeof(Slot));
      sw.set_name(name);
      sw.set_comments(trimmed(comments));

      // FIXME all the other values

      ++slot_number;
    }
    else {
      if (collect_comments)
        comments += line;
    }
  }
  file->close();

  memcpy(&set_list, &new_set_list, sizeof(SetList));
}

// Loads set list slot settings and returns original set list slot index.
int Editor::load_set_list_slot_settings_from_file(SlotWrapper &sw) {
  int orig_slot_number = 0;

  file->skip_table_headers();
  while (!file->is_table_separator()) {
    string setting_name = file->table_col1();
    if (setting_name == "Performance")
      sw.set_performance_name(file->table_col2());
    else if (setting_name == "Color")
      sw.set_color_name(file->table_col2());
    else if (setting_name == "Font")
      sw.set_font_name(file->table_col2());
    else if (setting_name == "Transpose")
      sw.set_xpose(atoi(file->table_col2().c_str()));
    else if (setting_name == "Volume")
      sw.set_volume(atoi(file->table_col2().c_str()));
    else if (setting_name == "Hold Time")
      sw.set_hold_time(atoi(file->table_col2().c_str()));
    else if (setting_name == "Kbd Track")
      sw.set_keyboard_track(atoi(file->table_col2().c_str()));
    file->gets();
  }

  file->gets();
  if (file->table_col1() == "Orig. Slot")
    orig_slot_number = atoi(file->table_col2().c_str());
  else
    fprintf(stderr, "error: \"Orig. Slot\" missing from data table\n");

  return orig_slot_number;
}

string Editor::trimmed(string s) {
  char buf[1024], *p;

  strncpy(buf, s.c_str(), 1023);
  buf[1023] = 0;
  for (p = buf; *p && isspace(*p); ++p) ;
  for (char *q = p + strlen(p) -1; q >= p && isspace(*q); --q)
    *q = 0;
  return string(p);
}

void Editor::write_slot() {
  KString *kstr;

  kstr = new KString(MD_INIT_INTERNAL, (byte *)name.c_str(),
                     SLOT_NAME_LEN, 0);
  kronos->write_current_slot_name(kstr);
  delete kstr;
  if (kronos->error_reply_seen()) // error already printed
    return;

  kstr = new KString(MD_INIT_INTERNAL, (byte *)comments.c_str(),
                     SLOT_COMMENTS_LEN, 0);
  kronos->write_current_slot_comments(kstr);
  delete kstr;
  if (kronos->error_reply_seen()) // error already printed
    return;
}
