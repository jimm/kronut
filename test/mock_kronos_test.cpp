#include <string.h>
#include "catch.hpp"
#include "test_helper.h"
#include "mock_kronos.h"

#define CATCH_CATEGORY "[mock-kronos]"

TEST_CASE("mk_get_name", CATCH_CATEGORY) {
  MockKronos mk = MockKronos(0);

  REQUIRE(strcmp(mk.read_current_slot_name()->str(), "Slot Name") == 0);
}

TEST_CASE("mk_get_comment", CATCH_CATEGORY) {
  MockKronos mk = MockKronos(0);

  REQUIRE(strcmp(mk.read_current_slot_comments()->str(),
                 "Slot Comments\nAnd another line") == 0);
}
