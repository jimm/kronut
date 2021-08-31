#include <string.h>
#include "catch.hpp"
#include "test_helper.h"
#include "../src/slot_wrapper.h"

#define CATCH_CATEGORY "[slot-wrapper]"

static const char * const test_name = "abcdefghijklmnopqrstuvwx";

TEST_CASE("slot wrapper accessors", CATCH_CATEGORY) {
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

    SECTION("set name truncation") {
      char buf[BUFSIZ];
      strcpy(buf, "this is too long so we should see a non-zero return from set_name");
      REQUIRE(strlen(buf) > SLOT_NAME_LEN); // sanity check

      REQUIRE(sw.set_name(buf) != 0);
      string returned = sw.name();
      buf[SLOT_NAME_LEN] = 0;
      REQUIRE(returned == buf);
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

    sw.set_font(font_s);
    REQUIRE(sw.font() == font_s);
  }

  SECTION("font name getters") {
    REQUIRE(strcmp(sw.font_name(), "Small") == 0);
    REQUIRE(strcmp(sw.font_short_name(), "S") == 0);

    sw.set_font(font_xs);
    REQUIRE(strcmp(sw.font_name(), "Extra Small") == 0);
    REQUIRE(strcmp(sw.font_short_name(), "XS") == 0);

    sw.set_font(font_m);
    REQUIRE(strcmp(sw.font_name(), "Medium") == 0);
    REQUIRE(strcmp(sw.font_short_name(), "M") == 0);

    sw.set_font(font_l);
    REQUIRE(strcmp(sw.font_name(), "Large") == 0);
    REQUIRE(strcmp(sw.font_short_name(), "L") == 0);

    sw.set_font(font_xl);
    REQUIRE(strcmp(sw.font_name(), "Extra Large") == 0);
    REQUIRE(strcmp(sw.font_short_name(), "XL") == 0);
  }

  SECTION("font names") {
    sw.set_font(font_s);
    REQUIRE(strcmp(sw.font_name(), "Small") == 0);

    sw.set_font(font_m);
    REQUIRE(strcmp(sw.font_name(), "Medium") == 0);

    sw.set_font(font_xl);
    REQUIRE(strcmp(sw.font_name(), "Extra Large") == 0);
  }

  SECTION("font name setters") {
    sw.set_font_name("Small");
    REQUIRE(sw.font() == font_s);
    sw.set_font_name("Extra Small");
    REQUIRE(sw.font() == font_xs);
    sw.set_font_name("Medium");
    REQUIRE(sw.font() == font_m);
    sw.set_font_name("Large");
    REQUIRE(sw.font() == font_l);
    sw.set_font_name("Extra Large");
    REQUIRE(sw.font() == font_xl);

    sw.set_font_name("S");
    REQUIRE(sw.font() == font_s);
    sw.set_font_name("XS");
    REQUIRE(sw.font() == font_xs);
    sw.set_font_name("M");
    REQUIRE(sw.font() == font_m);
    sw.set_font_name("L");
    REQUIRE(sw.font() == font_l);
    sw.set_font_name("XL");
    REQUIRE(sw.font() == font_xl);
  }

  SECTION("color names") {
    REQUIRE(strcmp(sw.color_name(), "Default") == 0);

    sw.set_color(color_olive);
    REQUIRE(strcmp(sw.color_name(), "Olive") == 0);
  }

  SECTION("color name setters") {
    sw.set_color_name("olive");
    REQUIRE(sw.color() == color_olive);

    sw.set_color_name("na");
    REQUIRE(sw.color() == color_navy);

    sw.set_color_name("LAV");
    REQUIRE(sw.color() == color_lavender);
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

  SECTION("performance names") {
    sw.set_performance_type(pt_combination);
    sw.set_performance_bank(0x12);
    sw.set_performance_index(3);
    REQUIRE(sw.performance_name() == "Combi USER-B 003");

    sw.set_performance_bank(0x19);
    REQUIRE(sw.performance_name() == "Combi USER-BB 003");

    sw.set_performance_bank(0x07);
    REQUIRE(sw.performance_name() == "Combi g(1) 004"); // nums +1

    sw.set_performance_bank(0x06);
    REQUIRE(sw.performance_name() == "Combi GM 004"); // nums +1

    sw.set_performance_type(pt_program);
    sw.set_performance_bank(0x12);
    sw.set_performance_index(3);
    REQUIRE(sw.performance_name() == "Prog USER-B 003");
  }

  SECTION("performance name saving") {
    sw.set_performance_name("Combination INT-E 023");
    REQUIRE(sw.performance_type() == pt_combination);
    REQUIRE(sw.performance_bank() == 0x04);
    REQUIRE(sw.performance_index() == 23);
    REQUIRE(sw.performance_name() == "Combi INT-E 023");

    // test case-insensitivity and prefix for type
    sw.set_performance_name("prog int-e 123");
    REQUIRE(sw.performance_type() == pt_program);
    REQUIRE(sw.performance_bank() == 0x04);
    REQUIRE(sw.performance_index() == 123);
    REQUIRE(sw.performance_name() == "Prog INT-E 123");

    sw.set_performance_name("Combi GM 1");
    REQUIRE(sw.performance_type() == pt_combination);
    REQUIRE(sw.performance_bank() == 0x06);
    REQUIRE(sw.performance_index() == 0); // GM and a few others offset by 1
    REQUIRE(sw.performance_name() == "Combi GM 001");
  }
}
