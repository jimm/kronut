#include "set_list_wrapper.h"

string SetListWrapper::name() {
  return chars_to_string(set_list.name, SET_LIST_NAME_LEN);
}

void SetListWrapper::set_name(string str) {
  string_to_chars(set_list.name, SET_LIST_NAME_LEN, str);
}
