#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include "kronos.h"

#define EDITOR_OK 0
#define EDITOR_ERROR -1
#define EDITOR_TOO_LONG -2

#define EDITOR_FORMAT_ORG_MODE 0
#define EDITOR_FORMAT_MARKDOWN 1

class SetListWrapper;
class SlotWrapper;
class SetListFile;

class Editor {
public:
  Editor(int format);

  int load_set_list_from_file(char *path);
  int save_set_list_to_file(char *path);

  SetList &set_list() { return _set_list; }

protected:
  int _set_list_number;
  SetList _set_list;
  SetListFile *_file;

  void save_set_list_settings_to_file(SetListWrapper &slw);
  void save_set_list_slot_settings_to_file(SlotWrapper &sw);

  void load_set_list_settings_from_file(SetListWrapper &slw);
  void load_set_list_slot_settings_from_file(SlotWrapper &sw);

  string trimmed(string s);
};

#endif /* EDITOR_H */
