#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mock_kronos.h"
#include "../src/consts.h"
#include "../src/midi_data.h"
#include "../src/kstring.h"
#include "../src/set_list.h"
#include "../src/utils.h"

#define MOCK_BANK 2
#define MOCK_PROG 12

static MockKronos *mock_kronos_instance;

MockKronos *MockKronos_instance() {
  return mock_kronos_instance;
}

// ================ allocation ================

// channel 1-15
MockKronos::MockKronos(byte chan) : Kronos(chan, 0, 0, 0), name(""), comments("") {
  mock_kronos_instance = this;
}

MockKronos::~MockKronos() {
}

KString * MockKronos::read_current_string(int obj_type, byte pad) {
  byte buf[512];

  memset(buf, 0, 512);
  if (obj_type == OBJ_TYPE_SET_LIST_SLOT_NAME) {
    strcpy((char *)buf, "Slot Name");
    return new KString(MD_INIT_INTERNAL, buf, SLOT_NAME_LEN, 0);
  }

  strcpy((char *)buf, "Slot Comments\nAnd another line");
  return new KString(MD_INIT_INTERNAL, buf, SLOT_COMMENTS_LEN, 0);
}
