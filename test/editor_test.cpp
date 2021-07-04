#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "catch.hpp"
#include "test_helper.h"
#include "../src/editor.h"
#include "../src/set_list_file.h"
#include "../src/mock_kronos.h"

#define CATCH_CATEGORY "[editor]"
#define SAVE_FILE "/tmp/kronut_editor.md"

// This subclass of Editor overrides `edit_file` to write text given to it.
class TestEditor : public Editor {
public:
  string saved_text;

  TestEditor() : Editor(EDITOR_FORMAT_MARKDOWN) {
    saved_text = "Slot Name\n\nSlot Comments\nAnd another line\n";
  }
};

// TODO

