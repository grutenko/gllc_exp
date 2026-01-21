#ifndef gllc_object_h
#define gllc_object_h

#include <stddef.h>

union gllc_variant {
  int bool_;
  int int_;
  double float_;
  char *string_;
  void *handle_;
};

enum {
  T_PROP_BOOL = 1,
  T_PROP_INT = 2,
  T_PROP_FLOAT = 3,
  T_PROP_STRING = 4,
  T_PROP_HANDLE = 5
};

struct gllc_prop_def {
  int prop;
  int type;
  union gllc_variant (*getter)(int, int);
  int (*setter)(int, int, union gllc_variant);
  int readonly;
};

struct gllc_prop_value {
  int prop;
  int type;
  union gllc_variant value;
};

struct gllc_object {
  const struct gllc_prop_def **prop_def;
  struct gllc_prop_value *prop_values;
  size_t prop_values_cap;
  size_t prop_values_size;
};

void gllc_object_cleanup(struct gllc_object *obj);
int gllc_prop_get_bool(struct gllc_object *obj, int prop);
int gllc_prop_get_int(struct gllc_object *obj, int prop);
double gllc_prop_get_float(struct gllc_object *obj, int prop);
const char *gllc_prop_get_string(struct gllc_object *obj, int prop);
void *gllc_prop_get_handle(struct gllc_object *obj, int prop);
int gllc_prop_put_bool(struct gllc_object *obj, int prop, int value);
int gllc_prop_put_int(struct gllc_object *obj, int prop, int value);
int gllc_prop_put_float(struct gllc_object *obj, int prop, double value);
int gllc_prop_put_string(struct gllc_object *obj, int prop, const char *value);
int gllc_prop_put_handle(struct gllc_object *obj, int prop, void *value);

#endif