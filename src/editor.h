#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include "kronos.h"
#include "kstring.h"

class Editor {
public:
  Editor(Kronos *k);
  ~Editor();

  // Reads current slot's name and comment, saves to temp file, opens
  // editor. When editor returns, reads temp file and writes current slot.
  void edit_current_slot();
  void print_current_slot();
  void dump_current_slot();

  // Read current slot's name and comment.
  void read_slot();

  // Write current slot's name and comment.
  void write_slot();

protected:
  Kronos *kronos;
  int set_number;
  int slot_number;
  KString *name;
  KString *comments;
  string save_dir;
  string curr_path;

  void save_to_file();
  virtual int edit_file();
  void load_from_file();
  void make_file_path();
};

#endif /* EDITOR_H */