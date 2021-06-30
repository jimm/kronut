#include <string.h>
#include "catch.hpp"
#include "test_helper.h"
#include "../src/slot_wrapper.h"

#define CATCH_CATEGORY "[slot-wrapper]"

static const char * const test_name = "abcdefghijklmnopqrstuvwx";

TEST_CASE("accessors", CATCH_CATEGORY) {
  Slot slot;
  SlotWrapper sw(slot);

  memset(&slot, 0, sizeof(Slot));

  SECTION("string handling") {
    SECTION("get name") {
      memcpy((void *)slot.name, (void *)test_name, (size_t)SLOT_NAME_LEN);
      REQUIRE(sw.name() == test_name);

      memset(slot.name, 0, SLOT_NAME_LEN);
      memcpy((void *)slot.name, (void *)"short", (size_t)5);
      REQUIRE(sw.name() == "short");
    }

    SECTION("set name") {
      sw.set_name(test_name);
      REQUIRE(memcmp(slot.name, test_name, SLOT_NAME_LEN) == 0);

      char buf[BUFSIZ];
      strcpy(buf, "short");
      memset(buf + 5, 0, SLOT_NAME_LEN); // overkill, yes
      sw.set_name(buf);
      REQUIRE(memcmp(slot.name, buf, SLOT_NAME_LEN) == 0);
    }

    SECTION("get comments") {
      memcpy((void *)slot.comments, (void *)test_name, (size_t)strlen(test_name));
      REQUIRE(sw.comments() == test_name);

      memset(slot.comments, 0, SLOT_COMMENTS_LEN);
      memcpy((void *)slot.comments, (void *)"short", (size_t)5);
      REQUIRE(sw.comments() == "short");
    }

    SECTION("set comments") {
      sw.set_comments(test_name);
      REQUIRE(memcmp(slot.comments, test_name, strlen(test_name)) == 0);
      REQUIRE(slot.comments[strlen(test_name)] == 0);

      char buf[BUFSIZ];
      strcpy(buf, "short");
      memset(buf + 5, 0, SLOT_COMMENTS_LEN); // overkill, yes
      sw.set_comments(buf);
      REQUIRE(memcmp(slot.comments, buf, SLOT_COMMENTS_LEN) == 0);
    }
  }

  SECTION("xpose reading") {
    slot.performance_bank = 0;
    slot.keyboard_track = 0;
    REQUIRE(sw.xpose() == 0);

    slot.performance_bank = 0;
    slot.keyboard_track = 0x20;
    REQUIRE(sw.xpose() == 1);

    slot.performance_bank = 0x60;
    slot.keyboard_track = 0;
    REQUIRE(sw.xpose() == 24);

    slot.performance_bank = 0xe0;
    slot.keyboard_track = 0xe0;
    REQUIRE(sw.xpose() == -1);

    slot.performance_bank = 0xa0;
    slot.keyboard_track = 0;
    REQUIRE(sw.xpose() == -24);
  }

  SECTION("xpose accessors") {
    sw.set_xpose(0);
    REQUIRE(sw.xpose() == 0);

    sw.set_xpose(24);
    REQUIRE(sw.xpose() == 24);

    sw.set_xpose(-1);
    REQUIRE(sw.xpose() == -1);

    sw.set_xpose(-24);
    REQUIRE(sw.xpose() == -24);
  }

  SECTION("font accessors") {
    REQUIRE(sw.font() == font_s); // the default zero value

    sw.set_font(font_xs);
    REQUIRE(sw.font() == font_xs);

    sw.set_font(font_m);
    REQUIRE(sw.font() == font_m);

    sw.set_font(font_l);
    REQUIRE(sw.font() == font_l);

    sw.set_font(font_xl);
    REQUIRE(sw.font() == font_xl);
  }

  SECTION("color names") {
    sw.set_color(color_default);
    REQUIRE(strcmp(sw.color_name(), "Default") == 0);

    sw.set_color(color_olive);
    REQUIRE(strcmp(sw.color_name(), "Olive") == 0);
  }

  SECTION("font names") {
    sw.set_font(font_s);
    REQUIRE(strcmp(sw.font_name(), "Small") == 0);

    sw.set_font(font_m);
    REQUIRE(strcmp(sw.font_name(), "Medium") == 0);

    sw.set_font(font_xl);
    REQUIRE(strcmp(sw.font_name(), "Extra Large") == 0);
  }

  SECTION("bank names") {
    sw.set_performance_bank(0x00);
    REQUIRE(sw.performance_bank_name() == "INT-A");

    sw.set_performance_bank(0x05);
    REQUIRE(sw.performance_bank_name() == "INT-F");

    sw.set_performance_bank(0x06);
    REQUIRE(sw.performance_bank_name() == "GM");

    sw.set_performance_bank(0x07);
    REQUIRE(sw.performance_bank_name() == "g(1)");

    sw.set_performance_bank(0x0f);
    REQUIRE(sw.performance_bank_name() == "g(9)");

    sw.set_performance_bank(0x10);
    REQUIRE(sw.performance_bank_name() == "g(d)");

    sw.set_performance_bank(0x11);
    REQUIRE(sw.performance_bank_name() == "USER-A");

    sw.set_performance_bank(0x17);
    REQUIRE(sw.performance_bank_name() == "USER-G");

    sw.set_performance_bank(0x18);
    REQUIRE(sw.performance_bank_name() == "USER-AA");

    sw.set_performance_bank(0x1e);
    REQUIRE(sw.performance_bank_name() == "USER-GG");
  }
}
