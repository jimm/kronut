#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include <string>
#include "kronos.h"
#include "kstring.h"

#define TEXT_EDITOR_OK 0
#define TEXT_EDITOR_ERROR -1
#define TEXT_EDITOR_TOO_LONG -2

class TextEditor {
public:
  TextEditor(Kronos &k);

  // Reads current slot's name and comment, saves to temp file, opens
  // editor. When editor returns, reads temp file and writes current slot.
  int edit_current_slot(bool read_from_kronos);

  string &current_name() { return name; }
  string &current_comments() { return comments; }
  bool name_too_long() { return name.length() > SLOT_NAME_LEN; }
  bool comments_too_long() { return comments.length() > SLOT_COMMENTS_LEN; }

  void print_current_slot();
  void dump_current_slot() { read_maybe_dump(true); }
  void print_set_list_slot_names();

  // Read current slot's name and comment.
  bool read_slot() { return read_maybe_dump(false); }

  // Write current slot's name and comment.
  void write_slot();

protected:
  Kronos &kronos;
  string name;
  string comments;

  bool read_maybe_dump(bool dump);
  void save_to_file();
  virtual int edit_file();
  void load_from_file();
  string trimmed(string s);
};

#endif /* TEXT_EDITOR_H */
