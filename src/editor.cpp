#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include "editor.h"

#define EDITOR_TMPFILE "/tmp/kronut_editor.md"
#define EDITOR_NAME 0
#define EDITOR_COMMENTS 1

Editor::Editor(Kronos *k) : kronos(k), name(""), comments("") {
  char *dir = getenv("KRONUT_EDIT_SAVE_DIR");
  if (dir) {
    save_dir = dir;
    if (dir[strlen(dir)-1] != '/')
      save_dir += '/';
  }
  else
    save_dir = string(getenv("HOME")) + "/kronut/";
}

int Editor::edit_current_slot(bool read_from_kronos) {
  if (read_from_kronos)
    read_slot();

  save_to_file();
  int status = edit_file();
  if (status == 0) {
    load_from_file();
    remove(EDITOR_TMPFILE);

    if (name_too_long() || comments_too_long())
      return EDITOR_TOO_LONG;

    write_slot();
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

void Editor::save_to_file() {
  FILE *fp = fopen(EDITOR_TMPFILE, "w");
  fprintf(fp, "# Slot Name\n\n");
  fprintf(fp, "%s\n\n", name.c_str());
  fprintf(fp, "# Comments\n\n");
  fprintf(fp, "%s\n", comments.c_str());
  fclose(fp);
}

int Editor::edit_file() {
  char buf[1024];
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
void Editor::load_from_file() {
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

string Editor::trimmed(string s) {
  char buf[1024], *p;

  strcpy(buf, s.c_str());
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
