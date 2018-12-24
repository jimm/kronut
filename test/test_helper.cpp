#include "test_helper.h"

byte midi_letters[28] = {
  0x40, 'a', 'b', 'c', 'd', 'e', 'f', 'g',
  0, 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  0, 'o', 'p', 'q', 'r', 's', 't', 'u',
  0, 'v', 'w', 'x'
};
byte internal_letters[24] = {
  'a', 'b', 'c', 'd', 'e', 'f', 'g' + 0x80,
  'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u',
  'v', 'w', 'x'
};
