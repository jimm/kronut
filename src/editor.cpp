#include "editor.h"

#define TMPFILE "/tmp/kronut.txt"


Editor::Editor(Kronos *k) : kronos(k), name(0), comments(0) {
}

Editor::~Editor() {
  if (name)
    delete name;
  if (comments)
    delete comments;
}

void Editor::edit_current_slot() {
  read_slot();
  save_to_tempfile();
  int status = edit_tempfile();
  if (status == 0) {
    load_from_tempfile();
    write_slot();
  }

  remove_tempfile();
}

void Editor::print_current_slot() {
  read_slot();
  save_to_tempfile();

  char buf[1024];
  sprintf(buf, "cat %s", TMPFILE);
  system(buf);
  puts("");
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
}

void Editor::save_to_tempfile() {
  FILE *fp = fopen(TMPFILE, "w");
  fprintf(fp, "%s\n\n", name->str()); // extra newline to make editing easier
  fputs(comments->str(), fp);
  fclose(fp);
}

int Editor::edit_tempfile() {
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
  sprintf(buf, "%s %s %s 2>&1", editor, options, TMPFILE);
  return system(buf);
}

void Editor::load_from_tempfile() {
  int chars_read;
  char buf[1024];

  FILE *fp = fopen(TMPFILE, "r");
  fgets(buf, name->internal_len+1, fp);
  if (buf[strlen(buf)-1] == '\n')
    buf[strlen(buf)-1] = 0;
  name->set_str(buf);

  fread(buf, 1, 1, fp); // skip newline added by read_slot()
  int len = fread(buf, 1, 1023, fp);
  buf[len--] = 0;
  while (len >= 0 && buf[len] == '\n') // strip trailing newlines
    buf[len--] = 0;
  comments->set_str(buf);
  fclose(fp);
}

void Editor::write_slot() {
  kronos->write_current_slot_name(name);
  if (kronos->error_reply_seen()) // error already printed
    return;

  kronos->write_current_slot_comments(comments);
  if (kronos->error_reply_seen()) // error already printed
    return;
}

void Editor::remove_tempfile() {
  remove(TMPFILE);
}
