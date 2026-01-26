#ifndef gllc_window_native_h
#define gllc_window_native_h

#if defined(_WIN32)
#include <windef.h>
#include <windows.h>
#endif

struct gllc_WN;

typedef void (*gllc_WN_paint_cb)(struct gllc_WN *w, void *USER_1);
typedef void (*gllc_WN_size_cb)(struct gllc_WN *w, int width, int height, void *USER_1);
typedef void (*gllc_WN_mouse_move_cb)(struct gllc_WN *w, int x, int y, void *USER_1);
typedef void (*gllc_WN_mouse_click_cb)(struct gllc_WN *wn, int x, int y, int mode, int action, void *USER_1);
typedef void (*gllc_WN_mouse_scroll_cb)(struct gllc_WN *wn, int dx, int dy, void *USER_1);

struct gllc_WN
{
#if defined(_WIN32)
        HWND w;
        HDC dc;
        HGLRC glrc;
#endif
        gllc_WN_paint_cb on_paint;
        gllc_WN_size_cb on_size;
        gllc_WN_mouse_move_cb on_mouse_move;
        gllc_WN_mouse_click_cb on_mouse_click;
        gllc_WN_mouse_scroll_cb on_mouse_scroll;

        void *on_paint_USER_1;
        void *on_size_USER_1;
        void *on_mouse_move_USER_1;
        void *on_mouse_click_USER_1;
        void *on_mouse_scroll_USER_1;

        struct gllc_WN *next;
        struct gllc_WN *prev;
};

struct gllc_WN *gllc_WN_create(void *parent);

void gllc_WN_on_paint(struct gllc_WN *w, gllc_WN_paint_cb on_paint, void *USER_1);

void gllc_WN_on_size(struct gllc_WN *w, gllc_WN_size_cb on_size, void *USER_1);

void gllc_WN_on_mouse_move(struct gllc_WN *w, gllc_WN_mouse_move_cb on_mouse_move, void *USER_1);

void gllc_WN_on_mouse_click(struct gllc_WN *w, gllc_WN_mouse_click_cb on_mouse_click, void *USER_1);

void gllc_WN_on_mouse_scroll(struct gllc_WN *w, gllc_WN_mouse_scroll_cb on_mouse_scroll, void *USER_1);

void gllc_WN_get_cursor(struct gllc_WN *w, int *x, int *y);

void gllc_WN_destroy(struct gllc_WN *w);

void gllc_WN_make_context_current(struct gllc_WN *w);

void gllc_WN_get_size(struct gllc_WN *w, int *width, int *height);

void gllc_WN_set_size(struct gllc_WN *w, int x, int y, int width, int height);

void gllc_WN_swap_buffers(struct gllc_WN *w);

void gllc_WN_dirty(struct gllc_WN *w);

#endif