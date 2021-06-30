#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <unistd.h>
#include "editor.h"
#include "set_list_wrapper.h"
#include "slot_wrapper.h"

#define EDITOR_TMPFILE "/tmp/kronut_editor.md"
#define EDITOR_NAME 0
#define EDITOR_COMMENTS 1

#define MARKDOWN_CHAR '#'
#define ORG_MODE_CHAR '*'

Editor::Editor(Kronos *k)
  : kronos(k), name(""), comments(""), header_char(MARKDOWN_CHAR)
{
  char *env_value = getenv("KRONUT_FILE_MODE");
  if (env_value != nullptr && (env_value[0] == 'o' || env_value[0] == 'O'))
    header_char = ORG_MODE_CHAR;
}

int Editor::edit_current_slot(bool read_from_kronos) {
  if (read_from_kronos)
    read_slot();

  save_slot_to_file();
  int status = edit_file();
  if (status == 0) {
    load_slot_from_file();
    remove(EDITOR_TMPFILE);

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
    remove(EDITOR_TMPFILE);

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
  FILE *fp = fopen(EDITOR_TMPFILE, "w");
  fprintf(fp, "%c Slot Name\n\n", header_char);
  fprintf(fp, "%s\n\n", name.c_str());
  fprintf(fp, "%c Comments\n\n", header_char);
  fprintf(fp, "%s\n", comments.c_str());
  fclose(fp);
}

void Editor::save_set_list_to_file() {
  SetListWrapper slw(set_list);

  FILE *fp = fopen(EDITOR_TMPFILE, "w");

  fprintf(fp, "%c %s\n\n", header_char, slw.name().c_str());

  for (int i = 0; i < 128; ++i) {
    Slot &slot = set_list.slots[i];
    SlotWrapper sw(slot);

    fprintf(fp, "%c%c %s\n\n", header_char, header_char, sw.name().c_str());

    fprintf(fp, "%c%c%c Comments \n\n", header_char, header_char, header_char);
    fprintf(fp, "%s\n\n", sw.comments().c_str());

    fprintf(fp, "%c%c%c Data\n\n", header_char, header_char, header_char);
    // FIXME
    fprintf(fp, "Original slot number: %d\n\n", i);
  }

  fclose(fp);
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
  sprintf(buf, "%s %s %s 2>&1", editor, options, EDITOR_TMPFILE);
  return system(buf);
}

// Loads name and comment from tempfile.
void Editor::load_slot_from_file() {
  int which = -1;
  string buf;
  char line[1024];

  FILE *fp = fopen(EDITOR_TMPFILE, "r");
  while (fgets(line, 1024, fp) != 0) {
    if (line[0] == header_char && strncmp(" Slot Name", line+1, 10) == 0) {
      which = EDITOR_NAME;
      buf = "";
    }
    else if (line[0] == header_char && strncmp(" Comments", line+1, 9) == 0) {
      which = EDITOR_COMMENTS;
      name = trimmed(buf);
      buf = "";
    }
    else if (which != -1)
      buf += line;
  }
  comments = trimmed(buf);
  fclose(fp);
}

void Editor::load_set_list_from_file() {
  char line[BUFSIZ];
  SetList new_set_list;
  SetListWrapper slw(new_set_list);
  long len;
  int slot_number = 0;

  memcpy((void *)&new_set_list, (void *)&set_list, sizeof(SetList));

  FILE *fp = fopen(EDITOR_TMPFILE, "r");
  while (fgets(line, BUFSIZ, fp) != 0) {
    if (line[0] == header_char && line[1] == ' ') {
      // Set List name
      name = trimmed(string(line + 2));
      slw.set_name(name);
    }
    else if (line[0] == header_char && line[1] == header_char && line[2] == ' ') {
      // Slot name, beginning of comments
      name = trimmed(string(line + 3));
      comments = "";
    }
    else if (strncmp("----", line, 4) == 0) {
      // noop
    }
    else if (strncmp("Original slot number: ", line, 22) == 0) {
      Slot &slot = new_set_list.slots[slot_number];
      SlotWrapper sw(slot);
      int orig_slot_number = atoi(line + 22);

      // copy original slot into this slot position
      memcpy((void *)&slot, (void *)&set_list.slots[orig_slot_number], sizeof(Slot));

      sw.set_name(name);
      sw.set_comments(trimmed(comments));

      // FIXME all the other values

      ++slot_number;
    }
    else {
      comments += line;
    }
  }
  fclose(fp);

  memcpy(&set_list, &new_set_list, sizeof(SetList));
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
