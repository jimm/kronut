#include "set_list_wrapper.h"

string SetListWrapper::name() {
  return chars_to_string(set_list.name, SET_LIST_NAME_LEN);
}

// Returns a non-zero value if `str` is too long.
int SetListWrapper::set_name(string str) {
  return string_to_chars(set_list.name, SET_LIST_NAME_LEN, str);
}

int SetListWrapper::slots_per_page() {
  return 16 >> set_list.slots_per_page;
}

void SetListWrapper::set_slots_per_page(int n) {
  switch (n) {
  case 16:
    set_list.slots_per_page = 0;
    break;
  case 8:
    set_list.slots_per_page = 1;
    break;
  case 4:
    set_list.slots_per_page = 2;
    break;
  }
}
