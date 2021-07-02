#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "catch.hpp"
#include "test_helper.h"
#include "../src/editor.h"
#include "../src/edit_file.h"
#include "../src/mock_kronos.h"

#define CATCH_CATEGORY "[editor]"
#define BUFSIZ 1024
#define SAVE_FILE "/tmp/kronut_editor.md"

// This subclass of Editor overrides `edit_file` to write text given to it.
class TestEditor : public Editor {
public:
  string saved_text;

  TestEditor(Kronos *k) : Editor(k) {
    saved_text = "Slot Name\n\nSlot Comments\nAnd another line\n";
    file = new MarkdownEditFile(); // force Markdown mode
  }

  int edit_file() {
    file->open("w");
    file->puts(saved_text.c_str());
    file->close();
    return 0;
  }

  const char *c_name() { return name.c_str(); }
  const char *c_comments() { return comments.c_str(); }

  void set_saved_text(string str) { saved_text = str; }
};

TEST_CASE("edit current slot saves", CATCH_CATEGORY) {
  char buf[BUFSIZ], *env_value;
  MockKronos mk = MockKronos(0);
  TestEditor ed = TestEditor(&mk);

  string file_text = "# Slot Name\n\nKronut Rules\n\n# Comments\n\nline one\nline B\n  \n \n";
  ed.set_saved_text(file_text);
  int status = ed.edit_current_slot(true);

  REQUIRE(status == EDITOR_OK);
  REQUIRE(strcmp(ed.c_name(), "Kronut Rules") == 0);
  REQUIRE(strcmp(ed.c_comments(), "line one\nline B") == 0);
}

TEST_CASE("edit text too long returns error", CATCH_CATEGORY) {
  MockKronos mk = MockKronos(0);
  TestEditor ed = TestEditor(&mk);

  string file_text = "# Slot Name\n\nKronut Rules\n\n# Comments\n\nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  ed.set_saved_text(file_text);
  int status = ed.edit_current_slot(true);
  REQUIRE(status == EDITOR_TOO_LONG);
}
