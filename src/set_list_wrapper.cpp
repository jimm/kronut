#include <iostream>
#include "set_list_wrapper.h"

const char * const CONTROL_SURFACE_ASSIGN_FROM_NAMES[2] = {
  "Slot", "Set List"
};

string SetListWrapper::name() {
  return chars_to_string(set_list.name, SET_LIST_NAME_LEN);
}

// Returns a non-zero value if `str` is too long.
int SetListWrapper::set_name(string str) {
  return string_to_chars(set_list.name, SET_LIST_NAME_LEN, str);
}

bool SetListWrapper::eq_bypass() {
  return (set_list.eq_bypass & 0x01) == 1;
}

void SetListWrapper::set_eq_bypass(bool val) {
  set_list.eq_bypass = (set_list.eq_bypass & 0xfe) + (val ? 1 : 0);
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

int SetListWrapper::control_surface_mode() {
  return set_list.control_surface_mode & 0x03;
}

void SetListWrapper::set_control_surface_mode(int n) {
  set_list.control_surface_mode = (set_list.control_surface_mode & 0xfc) + (n & 0x03);
}

SetListControlSurfaceAssignFrom SetListWrapper::control_surface_assign_from() {
  return (SetListControlSurfaceAssignFrom)(set_list.control_surface_assign_from & 0x01);
}

void SetListWrapper::set_control_surface_assign_from(SetListControlSurfaceAssignFrom val) {
  set_list.control_surface_assign_from = (set_list.control_surface_assign_from & 0xfe) + (int)(val & 0x01);
}

void SetListWrapper::set_control_surface_assign_from(string str) {
  SetListControlSurfaceAssignFrom val = (str[1] == 'e' || str[1] == 'E') ? cs_assign_set_list : cs_assign_slot;
  set_control_surface_assign_from(val);
}

const char * const SetListWrapper::control_surface_assign_from_name() {
  int index = (int)control_surface_assign_from();
  if (index < 0 || index >= (sizeof(CONTROL_SURFACE_ASSIGN_FROM_NAMES) / sizeof(const char * const))) {
    cerr << "error: illegal control surface assign from value " << index << endl;
    exit(1);
  }
  return CONTROL_SURFACE_ASSIGN_FROM_NAMES[index];
}
