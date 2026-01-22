#ifndef gllc_window_native_h
#define gllc_window_native_h

#if defined(_WIN32)
#include <windef.h>
#include <windows.h>
#endif

struct gllc_window_native {
#if defined(_WIN32)
  HWND w;
  HDC dc;
  HGLRC glrc;
#endif
};

struct gllc_window_native *gllc_WN_create(void *parent);
void gllc_WN_destroy(struct gllc_window_native *w);
void gllc_WN_make_context_current(struct gllc_window_native *w);
void gllc_WN_get_size(struct gllc_window_native *w, int *width, int *height);
void gllc_WN_swap_buffers(struct gllc_window_native *w);

#endif