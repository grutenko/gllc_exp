#include "glad.h"
#include "gllc_window_native.h"

#include <stdio.h>
#include <stdlib.h>
#include <winbase.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
                                                     const int *attribList);
wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList,
                                                 const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023

#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_TYPE_RGBA_ARB 0x202B

static int init_opengl_extensions(void)
{
        // Before we can load extensions, we need a dummy OpenGL context, created using a dummy window.
        // We use a dummy window because you can only set the pixel format for a window once. For the
        // real window, we want to use wglChoosePixelFormatARB (so we can potentially specify options
        // that aren't available in PIXELFORMATDESCRIPTOR), but we can't load and use that before we
        // have a context.
        WNDCLASSA window_class = {
            .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
            .lpfnWndProc = DefWindowProcA,
            .hInstance = GetModuleHandle(0),
            .lpszClassName = "Dummy_WGL_djuasiodwa",
        };

        if (!RegisterClassA(&window_class))
        {
                fprintf(stderr, "Failed to register dummy OpenGL window.");
                return 0;
        }

        HWND dummy_window = CreateWindowExA(
            0,
            window_class.lpszClassName,
            "Dummy OpenGL Window",
            0,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            window_class.hInstance,
            0);

        if (!dummy_window)
        {
                fprintf(stderr, "Failed to create dummy OpenGL window.");
                return 0;
        }

        HDC dummy_dc = GetDC(dummy_window);

        PIXELFORMATDESCRIPTOR pfd = {
            .nSize = sizeof(pfd),
            .nVersion = 1,
            .iPixelType = PFD_TYPE_RGBA,
            .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
            .cColorBits = 32,
            .cAlphaBits = 8,
            .iLayerType = PFD_MAIN_PLANE,
            .cDepthBits = 24,
            .cStencilBits = 8,
        };

        int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
        if (!pixel_format)
        {
                goto _error;
        }
        if (!SetPixelFormat(dummy_dc, pixel_format, &pfd))
        {
                goto _error;
        }

        HGLRC dummy_context = wglCreateContext(dummy_dc);
        if (!dummy_context)
        {
                goto _error;
        }

        if (!wglMakeCurrent(dummy_dc, dummy_context))
        {
                goto _error;
        }

        wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type *)wglGetProcAddress("wglCreateContextAttribsARB");
        wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type *)wglGetProcAddress("wglChoosePixelFormatARB");

        wglMakeCurrent(dummy_dc, 0);
        wglDeleteContext(dummy_context);
        ReleaseDC(dummy_window, dummy_dc);
        DestroyWindow(dummy_window);

        return 1;

_error:
        if (!dummy_window)
                return 0;
        if (dummy_dc)
                ReleaseDC(dummy_window, dummy_dc);
        if (dummy_context)
                wglDeleteContext(dummy_context);
        if (dummy_window)
                DestroyWindow(dummy_window);

        return 0;
}

static HGLRC init_opengl(HDC real_dc)
{
        if (!init_opengl_extensions())
        {
                return 0;
        }

        // Now we can choose a pixel format the modern way, using wglChoosePixelFormatARB.
        int pixel_format_attribs[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            0};

        int pixel_format;
        UINT num_formats;
        wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
        if (!num_formats)
        {
                fprintf(stderr, "Failed to set the OpenGL 3.3 pixel format.");
                return 0;
        }

        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat(real_dc, pixel_format, sizeof(pfd), &pfd);
        if (!SetPixelFormat(real_dc, pixel_format, &pfd))
        {
                fprintf(stderr, "Failed to set the OpenGL 3.3 pixel format.");
                return 0;
        }

        // Specify that we want to create an OpenGL 3.3 core profile context
        int gl33_attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB,
            3,
            WGL_CONTEXT_MINOR_VERSION_ARB,
            3,
            WGL_CONTEXT_PROFILE_MASK_ARB,
            WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0,
        };

        HGLRC gl33_context = wglCreateContextAttribsARB(real_dc, 0, gl33_attribs);
        if (!gl33_context)
        {
                fprintf(stderr, "Failed to create OpenGL 3.3 context.");
                return 0;
        }

        if (!wglMakeCurrent(real_dc, gl33_context))
        {
                fprintf(stderr, "Failed to activate OpenGL 3.3 rendering context.");
                wglDeleteContext(gl33_context);
                return 0;
        }

        return gl33_context;
}

static struct gllc_WN *G_window_head = NULL;
static struct gllc_WN *G_window_tail = NULL;

static void push_WN(struct gllc_WN *wn)
{
        if (!G_window_head)
                G_window_head = wn;

        wn->prev = G_window_tail;
        G_window_tail = wn;
}

static void remove_WN(struct gllc_WN *wn)
{
        if (wn->prev)
                wn->prev->next = wn->next;
        if (wn->next)
                wn->next->prev = wn->prev;
        if (!wn->prev)
                G_window_head = wn->next;
        if (!wn->next)
                G_window_tail = wn->prev;
}

static LRESULT CALLBACK window_callback(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
        LRESULT result = 0;

        struct gllc_WN *w = G_window_head;
        while (w)
        {
                if (w->w == window)
                        break;
                w = w->next;
        }

        if (w)
                gllc_WN_make_context_current(w);

        switch (msg)
        {
        case WM_PAINT:
                PAINTSTRUCT ps;
                BeginPaint(window, &ps);

                if (w && w->on_paint)
                        w->on_paint(w, w->on_paint_USER_1);

                EndPaint(window, &ps);

                return 0;
        case WM_LBUTTONDOWN:
                if (w && w->on_mouse_click)
                        w->on_mouse_click(w, LOWORD(lparam), HIWORD(lparam), 1, 1, w->on_mouse_click_USER_1);
                return 0;
        case WM_RBUTTONDOWN:
                if (w && w->on_mouse_click)
                        w->on_mouse_click(w, LOWORD(lparam), HIWORD(lparam), 2, 1, w->on_mouse_click_USER_1);
                return 0;
        case WM_MBUTTONDOWN:
                if (w && w->on_mouse_click)
                        w->on_mouse_click(w, LOWORD(lparam), HIWORD(lparam), 3, 1, w->on_mouse_click_USER_1);
                return 0;
        case WM_LBUTTONUP:
                if (w && w->on_mouse_click)
                        w->on_mouse_click(w, LOWORD(lparam), HIWORD(lparam), 1, 0, w->on_mouse_click_USER_1);
                return 0;
        case WM_RBUTTONUP:
                if (w && w->on_mouse_click)
                        w->on_mouse_click(w, LOWORD(lparam), HIWORD(lparam), 2, 0, w->on_mouse_click_USER_1);
                return 0;
        case WM_MBUTTONUP:
                if (w && w->on_mouse_click)
                        w->on_mouse_click(w, LOWORD(lparam), HIWORD(lparam), 3, 0, w->on_mouse_click_USER_1);
                return 0;
        case WM_SIZE:
                if (w)
                        glViewport(0, 0, LOWORD(lparam), HIWORD(lparam));

                if (w && w->on_size)
                        w->on_size(w, LOWORD(lparam), HIWORD(lparam), w->on_size_USER_1);

                return 0;
        case WM_MOUSEMOVE:
                if (w && w->on_mouse_move)
                        w->on_mouse_move(w, LOWORD(lparam), HIWORD(lparam), w->on_mouse_move_USER_1);

                return 0;
        case WM_MOUSEWHEEL:
                if (w && w->on_mouse_scroll)
                        w->on_mouse_scroll(w, 0, GET_WHEEL_DELTA_WPARAM(wparam) / 120, w->on_mouse_scroll_USER_1);
                return 0;

        case WM_MOUSEHWHEEL:
                if (w && w->on_mouse_scroll)
                        w->on_mouse_scroll(w, GET_WHEEL_DELTA_WPARAM(wparam) / 120, 0, w->on_mouse_scroll_USER_1);
                return 0;
        default:
                result = DefWindowProcA(window, msg, wparam, lparam);
                break;
        }

        return result;
}

ATOM wndClass = 0;

struct gllc_WN *gllc_WN_create(void *parent)
{
        struct gllc_WN *wn = malloc(sizeof(struct gllc_WN));
        if (wn)
        {
                memset(wn, 0, sizeof(struct gllc_WN));

                if (!wndClass)
                {
                        WNDCLASSEX wc = {};
                        wc.cbSize = sizeof(wc);
                        wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
                        wc.lpfnWndProc = window_callback;
                        wc.hInstance = GetModuleHandle(NULL);
                        wc.lpszClassName = "GLLCWindowClass";
                        wc.hCursor = LoadCursor(NULL, IDC_ARROW);

                        wndClass = RegisterClassEx(&wc);

                        if (!wndClass)
                                goto _error;
                }

                RECT rc;
                if (!GetClientRect((HWND)parent, &rc))
                {
                        rc.left = 0;
                        rc.top = 0;
                        rc.right = 800;
                        rc.bottom = 600;
                }

                wn->w = CreateWindowEx(
                    0,
                    MAKEINTATOM(wndClass),
                    "OpenGL Window",
                    WS_CHILD | WS_VISIBLE,
                    rc.left, rc.top,
                    rc.right - rc.left, rc.bottom - rc.top,
                    (HWND)parent,
                    NULL,
                    GetModuleHandle(NULL),
                    NULL);

                if (!wn->w)
                        goto _error;

                wn->dc = GetDC(wn->w);
                wn->glrc = init_opengl(wn->dc);

                if (!wn->glrc)
                        goto _error;

                push_WN(wn);
        }

        return wn;

_error:
        if (!wn)
                return 0;
        if (wn->dc)
                ReleaseDC(wn->w, wn->dc);
        if (wn->glrc)
                wglDeleteContext(wn->glrc);
        if (wn->w)
                DestroyWindow(wn->w);

        free(wn);

        return 0;
}

void gllc_WN_destroy(struct gllc_WN *w)
{
        if (!w)
                return;
        if (w->dc)
                ReleaseDC(w->w, w->dc);
        if (w->glrc)
                wglDeleteContext(w->glrc);
        if (w->w)
                DestroyWindow(w->w);

        remove_WN(w);
        free(w);
}

void gllc_WN_make_context_current(struct gllc_WN *w)
{
        if (w)
        {
                wglMakeCurrent(w->dc, w->glrc);
        }
        else
        {
                wglMakeCurrent(NULL, NULL);
        }
}

void gllc_WN_get_size(struct gllc_WN *w, int *width, int *height)
{
        *width = 0;
        *height = 0;

        RECT rc;
        if (GetClientRect(w->w, &rc))
        {
                *width = rc.right - rc.left;
                *height = rc.bottom - rc.top;
        }
}

void gllc_WN_get_cursor(struct gllc_WN *w, int *x, int *y)
{
        POINT p;

        GetCursorPos(&p);
        ScreenToClient((HWND)w->w, &p);

        if (x)
                *x = p.x;
        if (y)
                *y = p.y;
}

void gllc_WN_set_size(struct gllc_WN *w, int x, int y, int width, int height)
{
        SetWindowPos(w->w, NULL, x, y, width, height, SWP_NOZORDER);
}

void gllc_WN_on_paint(struct gllc_WN *w, gllc_WN_paint_cb on_paint, void *USER_1)
{
        w->on_paint = on_paint;
        w->on_paint_USER_1 = USER_1;
}

void gllc_WN_on_size(struct gllc_WN *w, gllc_WN_size_cb on_size, void *USER_1)
{
        w->on_size = on_size;
        w->on_size_USER_1 = USER_1;
}

void gllc_WN_on_mouse_move(struct gllc_WN *w, gllc_WN_mouse_move_cb on_mouse_move, void *USER_1)
{
        w->on_mouse_move = on_mouse_move;
        w->on_mouse_move_USER_1 = USER_1;
}

void gllc_WN_on_mouse_click(struct gllc_WN *w, gllc_WN_mouse_click_cb on_mouse_click, void *USER_1)
{
        w->on_mouse_click = on_mouse_click;
        w->on_mouse_click_USER_1 = USER_1;
}

void gllc_WN_on_mouse_scroll(struct gllc_WN *w, gllc_WN_mouse_scroll_cb on_mouse_scroll, void *USER_1)
{
        w->on_mouse_scroll = on_mouse_scroll;
        w->on_mouse_scroll_USER_1 = USER_1;
}

void gllc_WN_swap_buffers(struct gllc_WN *w)
{
        SwapBuffers(w->dc);
}

void gllc_WN_dirty(struct gllc_WN *w)
{
        InvalidateRect(w->w, NULL, TRUE);
}