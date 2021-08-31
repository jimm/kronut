#include "catch.hpp"
#include "test_helper.h"
#include "../src/set_list_wrapper.h"

#define CATCH_CATEGORY "[set-list-wrapper]"

TEST_CASE("set list wrapper accessors", CATCH_CATEGORY) {
  SetList set_list;
  SetListWrapper slw(set_list);

  SECTION("control surface assign") {
    slw.set_control_surface_assign_from(cs_assign_set_list);
    REQUIRE(slw.control_surface_assign_from() == cs_assign_set_list);
    REQUIRE(strcmp(slw.control_surface_assign_from_name(), "Set List") == 0);

    slw.set_control_surface_assign_from("setlist");
    REQUIRE(strcmp(slw.control_surface_assign_from_name(), "Set List") == 0);

    slw.set_control_surface_assign_from("Song");
    REQUIRE(slw.control_surface_assign_from() == cs_assign_slot);
    REQUIRE(strcmp(slw.control_surface_assign_from_name(), "Slot") == 0);
  }
}
