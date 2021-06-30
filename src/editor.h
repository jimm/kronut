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
  int edit_current_set_list(bool read_from_kronos);

  string &current_name() { return name; }
  string &current_comments() { return comments; }
  bool name_too_long() { return name.length() > SLOT_NAME_LEN; }
  bool comments_too_long() { return comments.length() > SLOT_COMMENTS_LEN; }

  void print_current_slot();
  void dump_current_slot() { read_maybe_dump(true); }
  void print_set_list_slot_names();
  void print_set_list_slot_values();

  // Read current slot's name and comment.
  void read_slot() { read_maybe_dump(false); }

  // Write current slot's name and comment.
  void write_slot();

  void set_file_type(int file_type);

protected:
  Kronos *kronos;
  string name;
  string comments;
  SetList set_list;
  char header_char;

  void read_maybe_dump(bool dump);

  void save_slot_to_file();
  void load_slot_from_file();

  void save_set_list_to_file();
  void load_set_list_from_file();

  virtual int edit_file();

  string trimmed(string s);

  bool is_header(int n, char *line);
};

#endif /* EDITOR_H */
