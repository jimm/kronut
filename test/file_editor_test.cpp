#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "catch.hpp"
#include "test_helper.h"
#include "mock_kronos.h"
#include "../src/file_editor.h"
#include "../src/set_list_file.h"

#define CATCH_CATEGORY "[file-editor]"

// This subclass of FileEditor overrides `edit_file` to write text given to it.
class TestEditor : public FileEditor {
public:
  string saved_text;

  TestEditor() : FileEditor(FILE_EDITOR_FORMAT_MARKDOWN) {
    saved_text = "Slot Name\n\nSlot Comments\nAnd another line\n";
  }
};

// TODO

