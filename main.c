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

struct gllc_window *w;

void update_colors(struct gllc_window *w)
{
        struct gllc_block *hBlock = gllc_window_get_block(w);
        struct gllc_block_entity *ent = gllc_block_get_first_ent(hBlock);
        while (ent)
        {
                unsigned int border_color = (rand() & 0xFF) << 16  // красный
                                            | (rand() & 0xFF) << 8 // зеленый
                                            | (rand() & 0xFF);     // синий

                // случайный цвет заливки
                unsigned int fill_color = (rand() & 0xFF) << 16 | (rand() & 0xFF) << 8 | (rand() & 0xFF);

                gllc_block_entity_set_color(ent, border_color);
                gllc_block_entity_set_fcolor(ent, fill_color);

                ent = gllc_block_entity_get_next(ent);
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
        //SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);

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
        int width = 600;
        int height = 600;
        for (int y = 0; y < height; y += grid_size)
        {
                for (int x = 0; x < width; x += grid_size)
                {
                        double x0 = (double)x - ((double)width / 2.0);
                        double y0 = (double)y - ((double)height / 2.0);
                        double x1 = x0 + (double)grid_size;
                        double y1 = y0 + (double)grid_size;
                        struct gllc_rect *hRect = gllc_block_add_rect(hBlock, x0, y0, x1, y1);
                        gllc_block_entity_set_color((struct gllc_block_entity *)hRect, 50 << 16 | 50 << 8 | 255);
                }
        }

        gllc_window_set_block(w, hBlock);
        gllc_block_update(hBlock);

        ShowWindow(hwnd, nCmdShow);

        MSG msg;
        int running = 1;

        while (running)
        {
                // Ждём, пока в очереди не появится сообщение
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