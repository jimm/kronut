#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include "editor.h"

#define EDITOR_TMPFILE "/tmp/kronut_editor.md"
#define EDITOR_NAME 0
#define EDITOR_COMMENTS 1

Editor::Editor(Kronos *k) : kronos(k), name(""), comments("") {
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
  char buf[BUFSIZ];

  kronos->read_current_set_list(set_list);
  memcpy(buf, set_list.name, SET_LIST_NAME_LEN);
  buf[SET_LIST_NAME_LEN] = '\0';
  printf("Set List: %s\n", buf);

  for (int i = 0; i < 128; ++i) {
    memcpy(buf, set_list.slots[i].name, SLOT_NAME_LEN);
    buf[SLOT_NAME_LEN] = '\0';
    printf("%3d\t%s\n", i + 1, buf);
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
  fprintf(fp, "# Slot Name\n\n");
  fprintf(fp, "%s\n\n", name.c_str());
  fprintf(fp, "# Comments\n\n");
  fprintf(fp, "%s\n", comments.c_str());
  fclose(fp);
}

void Editor::save_set_list_to_file() {
  FILE *fp = fopen(EDITOR_TMPFILE, "w");

  KString set_list_name(MD_INIT_INTERNAL, (byte *)&set_list.name, SET_LIST_NAME_LEN, 0);
  fprintf(fp, "# %s\n\n", set_list_name.str());

  for (int i = 0; i < 128; ++i) {
    Slot &slot = set_list.slots[i];

    KString slot_name(MD_INIT_INTERNAL, (byte *)&slot.name, SLOT_NAME_LEN, 0);
    fprintf(fp, "## %s\n\n", slot_name.str());

    KString slot_comments(MD_INIT_INTERNAL, (byte *)&slot.comments, SLOT_COMMENTS_LEN, 0);
    fprintf(fp, "%s\n\n", slot_comments.str());

    fprintf(fp, "----\n");
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
    if (strncmp("# Slot Name", line, 11) == 0) {
      which = EDITOR_NAME;
      buf = "";
    }
    else if (strncmp("# Comments", line, 10) == 0) {
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
  long len;
  int slot_number = 0;

  memcpy((void *)&new_set_list, (void *)&set_list, sizeof(SetList));

  FILE *fp = fopen(EDITOR_TMPFILE, "r");
  while (fgets(line, BUFSIZ, fp) != 0) {
    if (strncmp("# ", line, 2) == 0) {
      // Set List name
      name = trimmed(string(line + 2));
      memset(set_list.name, 0, SET_LIST_NAME_LEN);
      len = name.size();
      if (len >= SET_LIST_NAME_LEN) len = SET_LIST_NAME_LEN;
      memcpy(new_set_list.name, trimmed(name).c_str(), len);
    }
    else if (strncmp("## ", line, 3) == 0) {
      // Slot name, beginning of comments
      name = trimmed(string(line + 3));
      comments = "";
    }
    else if (strncmp("----", line, 4) == 0) {
      // noop
    }
    else if (strncmp("Original slot number: ", line, 22) == 0) {
      Slot &slot = new_set_list.slots[slot_number];
      int orig_slot_number = atoi(line + 22);

      // copy original slot into this slot position
      memcpy((void *)&slot, (void *)&set_list.slots[orig_slot_number], sizeof(Slot));

      // overwrite slot name
      memset(slot.name, 0, SLOT_NAME_LEN);
      len = name.size();
      if (len > SLOT_NAME_LEN) len = SLOT_NAME_LEN;
      memcpy(slot.name, name.c_str(), len);

      // overwrite comments
      KString kstr_comments(MD_INIT_INTERNAL, (byte *)"", SLOT_COMMENTS_LEN, 0);
      kstr_comments.set_str(trimmed(comments).c_str()); // handles newlines
      memcpy(slot.comments, kstr_comments.internal_bytes, SLOT_COMMENTS_LEN);

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
