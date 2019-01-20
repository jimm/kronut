#include <ctype.h>
#include <libgen.h>
#include "editor.h"

#define EDITOR_NAME 0
#define EDITOR_COMMENTS 1

Editor::Editor(Kronos *k) : kronos(k), name(0), comments(0) {
  char *dir = getenv("KRONUT_EDIT_SAVE_DIR");
  if (dir) {
    save_dir = dir;
    if (dir[strlen(dir)-1] != '/')
      save_dir += '/';
  }
  else
    save_dir = string(getenv("HOME")) + "/kronut/";
}

Editor::~Editor() {
  if (name)
    delete name;
  if (comments)
    delete comments;
}

void Editor::edit_current_slot() {
  read_slot();
  make_file_path();
  save_to_file();
  int status = edit_file();
  if (status == 0) {
    load_from_file();
    write_slot();
  }
}

void Editor::print_current_slot() {
  read_slot();
  puts(name->str());
  puts("");
  puts(comments->str());
}

void Editor::dump_current_slot() {
  name = kronos->read_current_slot_name();
  kronos->dump_sysex("slot name");
  comments = kronos->read_current_slot_comments();
  kronos->dump_sysex("slot comments");
}

void Editor::read_slot() {
  name = kronos->read_current_slot_name();
  comments = kronos->read_current_slot_comments();
  set_number = kronos->current_set_number();
  slot_number = kronos->current_slot_number();
}

void Editor::save_to_file() {
  FILE *fp = fopen(curr_path.c_str(), "w");
  fprintf(fp, "# Slot Name\n\n");
  fprintf(fp, "%s\n\n", name->str());
  fprintf(fp, "# Comments\n\n");
  fprintf(fp, "%s\n", comments->str());
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
  sprintf(buf, "%s %s %s 2>&1", editor, options, curr_path.c_str());
  return system(buf);
}

// Loads name and comment from tempfile.
void Editor::load_from_file() {
  int which = -1;
  string buf;
  char line[1024];

  FILE *fp = fopen(curr_path.c_str(), "r");
  while (fgets(line, 1024, fp) != 0) {
    if (strncmp("# Slot Name", line, 11) == 0) {
      which = EDITOR_NAME;
      buf = "";
    }
    else if (strncmp("# Comments", line, 10) == 0) {
      which = EDITOR_COMMENTS;
      name->set_str(trimmed(buf).c_str());
      buf = "";
    }
    else if (which != -1)
      buf += line;
  }
  comments->set_str(trimmed(buf).c_str());
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
  kronos->write_current_slot_name(name);
  if (kronos->error_reply_seen()) // error already printed
    return;

  kronos->write_current_slot_comments(comments);
  if (kronos->error_reply_seen()) // error already printed
    return;
}

// Save path to file into curr_path. Makes parent directories if needed.
void Editor::make_file_path() {
  char buf[1024];

  sprintf(buf, "%s%03d/%03d.md", save_dir.c_str(), set_number, slot_number);
  curr_path = buf;

  sprintf(buf, "mkdir -p %s", dirname((char *)curr_path.c_str()));
  system(buf);
}
