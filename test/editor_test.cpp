#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "test_helper.h"
#include "editor_test.h"
#include "../src/editor.h"
#include "../src/mock_kronos.h"

#define BUFSIZ 1024
#define SAVE_DIR "/tmp/kronut_tests"
#define SAVE_FILE SAVE_DIR "/002/012.txt"

void test_editor_setup() { system("rm -rf " SAVE_DIR); }
void test_editor_teardown() { system("rm -rf " SAVE_DIR); }

// This subclass of Editor overrides `edit_file` to write text given to it.
class TestEditor : public Editor {
public:
  string saved_text;

  TestEditor(Kronos *k) : Editor(k) {
    saved_text = "Slot Name\n\nSlot Comments\nAnd another line\n";
  }

  int edit_file() {
    system("cat " SAVE_FILE);
    FILE *fp = fopen(SAVE_FILE, "w");
    fprintf(fp, "%s", saved_text.c_str());
    fclose(fp);
    return 0;
  }

  void set_saved_text(string str) { saved_text = str; }
};

// Compare contents of file `path` to `contents`.
void assert_file_contents(string path, string contents, string where_from) {
  char buf[BUFSIZ];
  FILE *fp = fopen(path.c_str(), "r");
  size_t len = fread(buf, 1, BUFSIZ, fp);
  fclose(fp);
  buf[len] = 0;

  string err = where_from;
  err += ": contents mis-match in ";
  err += path;
  err += "; expected ";
  err += contents;
  err += " but got ";
  err += buf;
  tassert(contents == string(buf), err.c_str());
}

void test_ed_edit_current_slot_saves() {
  setenv("KRONUT_EDIT_SAVE_DIR", SAVE_DIR, 1);
  test_editor_setup();

  MockKronos mk = MockKronos(0);
  TestEditor ed = TestEditor(&mk);

  string file_text = "Slot Name\n\nSlot Comments\nAnd another line\n";
  ed.set_saved_text(file_text);
  ed.edit_current_slot();
  tassert(access(SAVE_FILE, F_OK) == 0, "file not saved to proper place");
  assert_file_contents(SAVE_FILE, file_text, "default save");
  test_editor_teardown();
}

void test_editor() {
  test_run(test_ed_edit_current_slot_saves);
}
