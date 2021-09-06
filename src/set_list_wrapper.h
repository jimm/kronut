#ifndef SET_LIST_WRAPPER_H
#define SET_LIST_WRAPPER_H

#include <string>
#include "set_list.h"
#include "struct_wrapper.h"

using namespace std;

typedef unsigned char byte;

enum SetListControlSurfaceAssignFrom {
  cs_assign_slot = 0,
  cs_assign_set_list
};

class SetListWrapper : public StructWrapper {
public:
  SetList &set_list;

  SetListWrapper(SetList &s) : set_list(s) {}

  string name();
  int set_name(string str);

  bool eq_bypass();
  void set_eq_bypass(bool val);

  int slots_per_page();
  void set_slots_per_page(int n);

  int control_surface_mode();
  void set_control_surface_mode(int n);

  SetListControlSurfaceAssignFrom control_surface_assign_from();
  void set_control_surface_assign_from(SetListControlSurfaceAssignFrom val);
  void set_control_surface_assign_from(string str);
  const char * const control_surface_assign_from_name();
};

#endif /* SLOT_WRAPPER_H */
