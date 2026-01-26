#ifndef gllc_window_native_h
#define gllc_window_native_h

#if defined(_WIN32)
#include <windef.h>
#include <windows.h>
#endif

struct gllc_WN;

typedef void (*gllc_WN_paint_cb)(struct gllc_WN *w, void *USER_1);

struct gllc_WN
{
#if defined(_WIN32)
        HWND w;
        HDC dc;
        HGLRC glrc;
#endif
        void *on_paint_USER_1;
        gllc_WN_paint_cb on_paint;

        struct gllc_WN *next;
        struct gllc_WN *prev;
};

struct gllc_WN *gllc_WN_create(void *parent);

void gllc_WN_on_paint(struct gllc_WN *w, gllc_WN_paint_cb on_paint, void *USER_1);

void gllc_WN_destroy(struct gllc_WN *w);

void gllc_WN_make_context_current(struct gllc_WN *w);

void gllc_WN_get_size(struct gllc_WN *w, int *width, int *height);

void gllc_WN_set_size(struct gllc_WN *w, int width, int height);

void gllc_WN_swap_buffers(struct gllc_WN *w);

void gllc_WN_dirty(struct gllc_WN *w);

#endif