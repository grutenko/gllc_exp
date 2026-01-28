#include "gllc_block.h"
#include "gllc_block_entity.h"
#include "gllc_drawing.h"
#include "gllc_polyline.h"
#include "gllc_window.h"
#include "include/gllc_circle.h"
#include "include/gllc_rect.h"

#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winuser.h>

#include <dwg.h>
#include <dwg_api.h>

struct gllc_window *w;

double rand_double(double min, double max)
{
        return min + ((double)rand() / (double)RAND_MAX) * (max - min);
}

void update_colors(struct gllc_window *w)
{
        struct gllc_block *hBlock = gllc_window_get_block(w);
        struct gllc_block_entity *ent = gllc_block_get_first_ent(hBlock);
        if (ent)
        {
                gllc_block_remove_ent(hBlock, ent);
                gllc_block_entity_destroy(ent);
        }

        gllc_block_update(hBlock);
        gllc_window_redraw(w);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        switch (uMsg)
        {
        case WM_TIMER:
                update_colors(w);
                return 0;
        case WM_SIZE:
                gllc_window_set_size(w, 0, 0, LOWORD(lParam), HIWORD(lParam));
                return 0;
        case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

const char *dwg_entity_type_to_string(int type)
{
        switch (type)
        {
        case DWG_TYPE_LINE:
                return "LINE";
        case DWG_TYPE_LWPOLYLINE:
                return "LWPOLYLINE";
        case DWG_TYPE_CIRCLE:
                return "CIRCLE";
        case DWG_TYPE_ARC:
                return "ARC";
        case DWG_TYPE_TEXT:
                return "TEXT";
        case DWG_TYPE_MTEXT:
                return "MTEXT";
        case DWG_TYPE_INSERT:
                return "INSERT";
        case DWG_TYPE_SOLID:
                return "SOLID";
        case DWG_TYPE_SHAPE:
                return "SHAPE";
        case DWG_TYPE_SPLINE:
                return "SPLINE";
        default:
                return "UNKNOWN";
        }
}

unsigned int rand_color()
{
        unsigned char r = rand() % 256;
        unsigned char g = rand() % 256;
        unsigned char b = rand() % 256;
        return (r << 16) | (g << 8) | b;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
        srand((unsigned int)time(NULL));

        const char CLASS_NAME[] = "MyWindowClass";

        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;

        RegisterClass(&wc);

        HWND hwnd = CreateWindowEx(
            0,
            CLASS_NAME,
            "SimpleWindow",
            WS_OVERLAPPEDWINDOW,

            CW_USEDEFAULT, CW_USEDEFAULT,
            800, 600,

            NULL,
            NULL,
            hInstance,
            NULL);

#define TIMER_ID 1
#define TIMER_INTERVAL 64

        // после создания окна:
        // SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);

        w = gllc_window_create(hwnd);
        if (!w)
        {
                return EXIT_FAILURE;
        }
        gllc_window_set_clear_color(w, 0, 0, 0);
        float color[4] = {
            0.2f,
            0.2f,
            0.2f,
            1.0f,
        };
        gllc_window_grid_configure(w, 20.0f, 20.0f, 0.0f, 0.0f, color);

        struct gllc_drawing *hDrw = gllc_drawing_create();
        struct gllc_block *hBlock = gllc_drw_add_block(hDrw, "Model Space", 0.0f, 0.0f);

        int grid_size = 2;
        int width = 400;
        int height = 400;
        for (int y = 0; y < height; y += grid_size)
        {
                for (int x = 0; x < width; x += grid_size)
                {
                        double x0 = (double)x - ((double)width / 2.0);
                        double y0 = (double)y - ((double)height / 2.0);
                        double x1 = x0 + (double)grid_size;
                        double y1 = y0 + (double)grid_size;
                        struct gllc_rect *hRect = gllc_block_add_rect(hBlock, x0, y0, x1, y1);
                        gllc_block_entity_set_color((struct gllc_block_entity *)hRect, 255 << 16 | 255 << 8 | 255);
                }
        }

        double x_min = -1000.0, x_max = 1000.0;
        double y_min = -1000.0, y_max = 1000.0;
        double r_min = 0.5, r_max = 2.0;
        int i;
        for (i = 0; i < 90000; i++)
        {
                double x = rand_double(x_min, x_max);
                double y = rand_double(y_min, y_max);
                double radius = rand_double(r_min, r_max);
                struct gllc_circle *hCircle = gllc_block_add_circle(hBlock, x, y, radius);
                gllc_block_entity_set_color((struct gllc_block_entity *)hCircle, rand_color());
        }

        gllc_window_set_block(w, hBlock);
        gllc_block_update(hBlock);

        ShowWindow(hwnd, nCmdShow);

        MSG msg;
        int running = 1;

        while (running)
        {
                WaitMessage();

                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                        if (msg.message == WM_QUIT)
                        {
                                running = 0;
                                break;
                        }

                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                }
        }

        gllc_window_destroy(w);

        return 0;
}