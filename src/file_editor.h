#ifndef FILE_EDITOR_H
#define FILE_EDITOR_H

#include <string>
#include "kronos.h"

#define FILE_EDITOR_OK 0
#define FILE_EDITOR_ERROR -1
#define FILE_EDITOR_TOO_LONG -2

#define FILE_EDITOR_FORMAT_ORG_MODE 0
#define FILE_EDITOR_FORMAT_MARKDOWN 1
#define FILE_EDITOR_FORMAT_HEXDUMP 2

class SetListWrapper;
class SlotWrapper;
class SetListFile;

class FileEditor {
public:
  FileEditor(int format);
  ~FileEditor();

  int load_set_list_from_file(const char * const path);
  int save_set_list_to_file(const char * const path, bool skip_empty_slots);

  SetList &set_list() { return _set_list; }

protected:
  int _file_format;
  int _set_list_number;
  SetList _set_list;
  SetListFile *_file;

  void save_set_list_settings_to_file(SetListWrapper &slw);
  void save_set_list_slot_settings_to_file(SlotWrapper &sw);

  void load_set_list_settings_from_file(SetListWrapper &slw);
  void load_set_list_slot_settings_from_file(SlotWrapper &sw);

  int hexdump(const char * const path);

  string trimmed(string s);
  void init_set_list();
};

#endif /* FILE_EDITOR_H */
