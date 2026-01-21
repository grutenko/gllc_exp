#ifndef gllc_window_h
#define gllc_window_h

#include "gllc_object.h"

struct gllc_window_native;
struct gllc_block;

struct gllc_window {
  struct gllc_object __obj;
  struct gllc_window_native *native;
  struct gllc_block *block;
};

struct gllc_window *gllc_window_create(void *parent_handle);
void gllc_window_set_block(struct gllc_window *window,
                           struct gllc_block *block);
void gllc_window_destroy(struct gllc_window *window);

#endif