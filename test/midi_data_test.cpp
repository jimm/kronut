#include <string.h>
#include "test_helper.h"
#include "midi_data_test.h"

void test_md_m2i_len() {
  MIDIData md(MD_INIT_MIDI, midi_letters, 28);
  tassert(md.internal_len == 24, "bad m2i len conversion");
}

void test_md_i2m_len() {
  MIDIData md(MD_INIT_INTERNAL, internal_letters, 24);
  tassert(md.midi_len == 28, "bad i2m len conversion");
}

void test_md_init_with_midi() {
  MIDIData md(MD_INIT_MIDI, midi_letters, 28);
  tassert(memcmp(md.internal_bytes, internal_letters, 24) == 0, "bad m2i conversion");
}

void test_md_init_with_internal() {
  MIDIData md(MD_INIT_INTERNAL, internal_letters, 28);
  tassert(memcmp(md.midi_bytes, midi_letters, 24) == 0, "bad i2m conversion");
}

void test_midi_data() {
  test_run(test_md_m2i_len);
  test_run(test_md_i2m_len);
  test_run(test_md_init_with_midi);
  test_run(test_md_init_with_internal);
}
