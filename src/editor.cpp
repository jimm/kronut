#include <libgen.h>
#include "editor.h"

Editor::Editor(Kronos *k) : kronos(k), name(0), comments(0) {
  char *dir = getenv("KRONUT_EDIT_SAVE_DIR");
  save_dir = dir || "/tmp/";
  if (dir != 0 && dir[strlen(dir)-1] != '/')
    save_dir += '/';
}

Editor::~Editor() {
  if (name)
    delete name;
  if (comments)
    delete comments;
}

void Editor::edit_current_slot() {
  read_slot();
  make_file_path("");
  save_to_file();
  int status = edit_file();
  if (status == 0) {
    bool truncated = load_from_file();
    write_slot();
    if (truncated)
      make_file_copy();
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
  fprintf(fp, "%s\n\n", name->str()); // extra newline to make editing easier
  fputs(comments->str(), fp);
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

// Loads name and comment from tempfile. Returns true if either name or
// comment was truncated.
bool Editor::load_from_file() {
  int chars_read;
  char buf[1024];
  bool truncated = false;

  FILE *fp = fopen(curr_path.c_str(), "r");
  fgets(buf, name->internal_len+1, fp);
  if (buf[strlen(buf)-1] == '\n')
    buf[strlen(buf)-1] = 0;
  name->set_str(buf);
  if (strcmp(name->str(), buf) != 0)
    truncated = true;

  fread(buf, 1, 1, fp); // skip newline added by read_slot()
  int len = fread(buf, 1, 1023, fp);
  buf[len--] = 0;
  while (len >= 0 && buf[len] == '\n') // strip trailing newlines
    buf[len--] = 0;
  comments->set_str(buf);
  if (strcmp(comments->str(), buf) != 0)
    truncated = true;

  fclose(fp);
  return truncated;
}

void Editor::write_slot() {
  kronos->write_current_slot_name(name);
  if (kronos->error_reply_seen()) // error already printed
    return;

  kronos->write_current_slot_comments(comments);
  if (kronos->error_reply_seen()) // error already printed
    return;
}

// Makes a copy of dirname(curr_path)/NNN.txt into
// dirname(curr_path)/NNN_original.txt.
void Editor::make_file_copy() {
  string edited_path = curr_path;
  char buf[1024];

  make_file_path("_original");
  sprintf(buf, "cp %s %s", edited_path.c_str(), curr_path.c_str());
  system(buf);
  curr_path = edited_path;
}

// Save path to file into curr_path. Makes parent directories if needed.
void Editor::make_file_path(const char * const suffix) {
  char buf[1024];

  sprintf(buf, "%s/%03d/%03d%s.txt", save_dir.c_str(), set_number, slot_number,
          suffix);
  curr_path = buf;

  sprintf(buf, "mkdir -p %s", dirname((char *)curr_path.c_str()));
  system(buf);
}
