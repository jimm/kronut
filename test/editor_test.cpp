#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "test_helper.h"
#include "editor_test.h"
#include "../src/editor.h"
#include "../src/mock_kronos.h"

#define BUFSIZ 1024
#define SAVE_FILE "/tmp/kronut_editor.md"

// This subclass of Editor overrides `edit_file` to write text given to it.
class TestEditor : public Editor {
public:
  string saved_text;

  TestEditor(Kronos *k) : Editor(k) {
    saved_text = "Slot Name\n\nSlot Comments\nAnd another line\n";
  }

  int edit_file() {
    FILE *fp = fopen(SAVE_FILE, "w");
    fprintf(fp, "%s", saved_text.c_str());
    fclose(fp);
    return 0;
  }

  const char *c_name() { return name.c_str(); }
  const char *c_comments() { return comments.c_str(); }

  void set_saved_text(string str) { saved_text = str; }
};

void test_ed_edit_current_slot_saves() {
  MockKronos mk = MockKronos(0);
  TestEditor ed = TestEditor(&mk);

  string file_text = "# Slot Name\n\nKronut Rules\n\n# Comments\n\nline one\nline B\n  \n \n";
  ed.set_saved_text(file_text);
  int status = ed.edit_current_slot(true);
  tassert(status == EDITOR_OK, 0);
  tassert(strcmp(ed.c_name(), "Kronut Rules") == 0,
          (string("name seen: \"") + ed.c_name() + "\"").c_str());
  tassert(strcmp(ed.c_comments(), "line one\nline B") == 0,
          (string("comments seen: \"") + ed.c_comments() + "\"").c_str());
}

void test_ed_too_long_returns_error() {
  MockKronos mk = MockKronos(0);
  TestEditor ed = TestEditor(&mk);

  string file_text = "# Slot Name\n\nKronut Rules\n\n# Comments\n\nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  ed.set_saved_text(file_text);
  int status = ed.edit_current_slot(true);
  char buf[BUFSIZ];
  sprintf(buf, "expected error return %d but got %d\n", EDITOR_TOO_LONG, status);
  tassert(status == EDITOR_TOO_LONG, buf);
}

void test_editor() {
  test_run(test_ed_edit_current_slot_saves);
  test_run(test_ed_too_long_returns_error);
}
