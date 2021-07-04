#ifndef SET_LIST_WRAPPER_H
#define SET_LIST_WRAPPER_H

#include <string>
#include "set_list.h"
#include "struct_wrapper.h"

using namespace std;

typedef unsigned char byte;

class SetListWrapper : public StructWrapper {
public:
  SetList &set_list;

  SetListWrapper(SetList &s) : set_list(s) {}

  string name();
  void set_name(string str);

  int slots_per_page();
  void set_slots_per_page(int n);
};

#endif /* SLOT_WRAPPER_H */
