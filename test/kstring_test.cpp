#include <string.h>
#include <catch2/catch_test_macros.hpp>
#include "test_helper.h"
#include "../src/kstring.h"

#define CATCH_CATEGORY "[kstring]"

TEST_CASE("ks_m2i_len", CATCH_CATEGORY) {
  KString kstr(MD_INIT_MIDI, midi_letters, 28, 0);
  REQUIRE(kstr.internal_len == 24);
}

TEST_CASE("ks_i2m_len", CATCH_CATEGORY) {
  KString kstr(MD_INIT_INTERNAL, internal_letters, 24, 0);
  REQUIRE(kstr.midi_len == 28);
}

TEST_CASE("ks_init_with_midi", CATCH_CATEGORY) {
  KString kstr(MD_INIT_MIDI, midi_letters, 28, 0);
  REQUIRE(memcmp(kstr.internal_bytes, internal_letters, 24) == 0);
}

TEST_CASE("ks_init_with_internal", CATCH_CATEGORY) {
  KString kstr(MD_INIT_INTERNAL, internal_letters, 28, 0);
  REQUIRE(memcmp(kstr.midi_bytes, midi_letters, 24) == 0);
}

TEST_CASE("ks_padding", CATCH_CATEGORY) {
  internal_letters[2] = '\r';
  internal_letters[22] = ' ';
  internal_letters[23] = ' ';
  KString kstr(MD_INIT_INTERNAL, internal_letters, 24, ' ');
  internal_letters[2] = 'c';
  internal_letters[22] = 'w';
  internal_letters[23] = 'x';
  char *c_str = kstr.str();
  REQUIRE(c_str[2] == '\n');
  REQUIRE(c_str[22] == 0);
  REQUIRE(c_str[23] == 0);
}

TEST_CASE("ks_truncation", CATCH_CATEGORY) {
  KString kstr(MD_INIT_MIDI, midi_letters, 28, 0);
  kstr.set_str("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
  REQUIRE(kstr.internal_len == 24);
  REQUIRE(kstr.midi_len == 28);
  REQUIRE(strcmp(kstr.str(), "abcdefghijklmnopqrstuvwx") == 0);
}

TEST_CASE("ks_set_str", CATCH_CATEGORY) {
  KString kstr(MD_INIT_INTERNAL, internal_letters, 24, ' ');
  kstr.set_str("hello\nworld");
  REQUIRE(strcmp(kstr.str(), "hello\nworld") == 0);
  byte *ibytes = kstr.internal_bytes;
  REQUIRE(ibytes[0] == 'h');
  REQUIRE(ibytes[5] == '\r');
  REQUIRE(ibytes[11] == ' ');
  REQUIRE(ibytes[23] == ' ');
}

TEST_CASE("ks_crlf", CATCH_CATEGORY) {
  byte midi_str[] = {
    0, 'a', 'b', 'c', 0x0d, 0x0a, 'd', 'e'
  };
  KString kstr(MD_INIT_MIDI, midi_str, 8, 0);
  REQUIRE(strcmp(kstr.str(), "abc\nde") == 0);
}
