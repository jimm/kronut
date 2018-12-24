#include <string.h>
#include "test_helper.h"
#include "kstring_test.h"

void test_ks_m2i_len() {
  KString kstr(MD_INIT_MIDI, midi_letters, 28, 0);
  tassert(kstr.internal_len == 24, "bad m2i len conversion");
}

void test_ks_i2m_len() {
  KString kstr(MD_INIT_INTERNAL, internal_letters, 24, 0);
  tassert(kstr.midi_len == 28, "bad i2m len conversion");
}

void test_ks_init_with_midi() {
  KString kstr(MD_INIT_MIDI, midi_letters, 28, 0);
  tassert(memcmp(kstr.internal_bytes, internal_letters, 24) == 0, "bad m2i conversion");
}

void test_ks_init_with_internal() {
  KString kstr(MD_INIT_INTERNAL, internal_letters, 28, 0);
  tassert(memcmp(kstr.midi_bytes, midi_letters, 24) == 0, "bad i2m conversion");
}

void test_ks_padding() {
  internal_letters[2] = '\r';
  internal_letters[22] = ' ';
  internal_letters[23] = ' ';
  KString kstr(MD_INIT_INTERNAL, internal_letters, 24, ' ');
  internal_letters[2] = 'c';
  internal_letters[22] = 'w';
  internal_letters[23] = 'x';
  char *c_str = kstr.str();
  tassert(c_str[2] = '\n', "line ending converted incorrectly");
  tassert(c_str[22] == 0, "padding not converted to 0 at 22");
  tassert(c_str[23] == 0, "padding not converted to 0 at 23");
}

void test_ks_set_str() {
  KString kstr(MD_INIT_INTERNAL, internal_letters, 24, ' ');
  kstr.set_str("hello\nworld");
  tassert(strcmp(kstr.str(), "hello\nworld") == 0, "bad str copy");
  byte *ibytes = kstr.internal_bytes;
  tassert(ibytes[0] == 'h', 0);
  tassert(ibytes[5] == '\r', "bad newline conversion");
  tassert(ibytes[11] == ' ', "bad padding at end of string");
  tassert(ibytes[23] == ' ', "bad padding at end of bytes");
}

void test_kstring() {
  test_run(test_ks_m2i_len);
  test_run(test_ks_i2m_len);
  test_run(test_ks_init_with_midi);
  test_run(test_ks_init_with_internal);
  test_run(test_ks_padding);
  test_run(test_ks_set_str);
}
