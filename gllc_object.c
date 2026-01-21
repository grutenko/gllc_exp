#include "gllc_object.h"
#include <stdlib.h>

void gllc_object_cleanup(struct gllc_object *obj) {
  int i;
  for (i = 0; i < obj->prop_values_size; i++) {
    if (obj->prop_values[i].type == T_PROP_STRING) {
      free(obj->prop_values[i].value.string_);
    }
  }
  free(obj->prop_values);
}