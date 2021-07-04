#include "set_list_wrapper.h"

string SetListWrapper::name() {
  return chars_to_string(set_list.name, SET_LIST_NAME_LEN);
}

void SetListWrapper::set_name(string str) {
  string_to_chars(set_list.name, SET_LIST_NAME_LEN, str);
}

int SetListWrapper::slots_per_page() {
  switch (set_list.reserved[0]) {
  case 1:
    return 8;
  case 2:
    return 4;
  case 0:
  default:
    return 16;
  }
  return 16;
}

void SetListWrapper::set_slots_per_page(int n) {
  switch (n) {
  case 8:
    set_list.reserved[0] = 1;
    break;
  case 4:
    set_list.reserved[0] = 2;
    break;
  case 16:
  default:
    set_list.reserved[0] = 0;
    break;
  }
}
