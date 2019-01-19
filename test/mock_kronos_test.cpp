#include <string.h>
#include "test_helper.h"
#include "../src/mock_kronos.h"
#include "mock_kronos_test.h"

void test_mk_get_name() {
  MockKronos mk = MockKronos(0);

  tassert(strcmp(mk.read_current_slot_name()->str(), "Slot Name") == 0, 0);
}

void test_mk_get_comment() {
  MockKronos mk = MockKronos(0);

  tassert(strcmp(mk.read_current_slot_comments()->str(),
                 "Slot Comments\nAnd another line") == 0,
          0);
}

void test_mock_kronos() {
  test_run(test_mk_get_name);
  test_run(test_mk_get_comment);
}
