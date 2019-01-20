#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include "kronos.h"
#include "kstring.h"

#define EDITOR_OK 0
#define EDITOR_ERROR -1
#define EDITOR_TOO_LONG -2

class Editor {
public:
  Editor(Kronos *k);

  // Reads current slot's name and comment, saves to temp file, opens
  // editor. When editor returns, reads temp file and writes current slot.
  int edit_current_slot(bool read_from_kronos);

  string &current_name() { return name; }
  string &current_comments() { return comments; }
  bool name_too_long() { return name.length() > SLOT_NAME_LEN; }
  bool comments_too_long() { return comments.length() > SLOT_COMMENTS_LEN; }

  void print_current_slot();
  void dump_current_slot() { read_maybe_dump(true); }

  // Read current slot's name and comment.
  void read_slot() { read_maybe_dump(false); }

  // Write current slot's name and comment.
  void write_slot();

protected:
  Kronos *kronos;
  string name;
  string comments;
  string save_dir;

  void read_maybe_dump(bool dump);
  void save_to_file();
  virtual int edit_file();
  void load_from_file();
  string trimmed(string s);
};

#endif /* EDITOR_H */
