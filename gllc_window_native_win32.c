#include "gllc_window_native.h"

#include <windows.h>

struct gllc_window_native *gllc_WN_create(void *parent) { return (void *)0; }

void gllc_WN_destroy(struct gllc_window_native *w) {
  if (!w)
    return;
  free(w);
}

void gllc_WN_make_context_current(struct gllc_window_native *w) {}