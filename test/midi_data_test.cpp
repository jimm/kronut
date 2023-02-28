#include <string.h>
#include <catch2/catch_test_macros.hpp>
#include "test_helper.h"
#include "../src/midi_data.h"

#define CATCH_CATEGORY "[midi-data]"

TEST_CASE("md_m2i_len", CATCH_CATEGORY) {
  MIDIData md(MD_INIT_MIDI, midi_letters, 28);
  REQUIRE(md.internal_len == 24);
}

TEST_CASE("md_i2m_len", CATCH_CATEGORY) {
  MIDIData md(MD_INIT_INTERNAL, internal_letters, 24);
  REQUIRE(md.midi_len == 28);
}

TEST_CASE("md_init_with_midi", CATCH_CATEGORY) {
  MIDIData md(MD_INIT_MIDI, midi_letters, 28);
  REQUIRE(memcmp(md.internal_bytes, internal_letters, 24) == 0);
}

TEST_CASE("md_init_with_internal", CATCH_CATEGORY) {
  MIDIData md(MD_INIT_INTERNAL, internal_letters, 28);
  REQUIRE(memcmp(md.midi_bytes, midi_letters, 24) == 0);
}
