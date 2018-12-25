#ifndef EDITOR_H
#define EDITOR_H

#include "kronos.h"
#include "kstring.h"

class Editor {
public:
  Editor(Kronos *k);
  ~Editor();

  // Reads current slot's name and comment, saves to temp file, opens
  // editor. When editor returns, reads temp file and writes current slot.
  void edit_current_slot();
  void dump_current_slot();

  // Read current slot's name and comment.
  void read_slot();

  // Write current slot's name and comment.
  void write_slot();

private:
  Kronos *kronos;
  KString *name;
  KString *comments;

  void save_to_tempfile();
  int edit_tempfile();
  void load_from_tempfile();
  void remove_tempfile();
};

#endif /* EDITOR_H */