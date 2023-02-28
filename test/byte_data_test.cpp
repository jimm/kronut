#include <string.h>
#include <catch2/catch_test_macros.hpp>
#include "test_helper.h"
#include "../src/byte_data.h"

#define CATCH_CATEGORY "[byte-data]"

TEST_CASE("append", CATCH_CATEGORY) {
  ByteData bd;
  REQUIRE(bd.size() == 0);
  for (int i = 0; i < 3; ++i)
    bd.append('a' + i);
  REQUIRE(bd.size() == 3);
  for (int i = 0; i < 3; ++i)
    REQUIRE(bd[i] == 'a' + i);
}

TEST_CASE("append with length", CATCH_CATEGORY) {
  ByteData bd;
  bd.append((byte *)"abcdefg", 7);
  REQUIRE(bd.size() == 7);
  for (int i = 0; i < 7; ++i)
    REQUIRE(bd[i] == 'a' + i);
}

TEST_CASE("clear", CATCH_CATEGORY) {
  ByteData bd;
  REQUIRE(bd.size() == 0);
  for (int i = 0; i < 3; ++i)
    bd.append('a' + i);
  bd.clear();
  REQUIRE(bd.size() == 0);
}

TEST_CASE("expansion", CATCH_CATEGORY) {
  ByteData bd;
  bd.append(midi_letters, 3000); // any address in memory
  REQUIRE(bd.size() == 3000);
}
