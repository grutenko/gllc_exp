#ifndef gllc_window_h
#define gllc_window_h

#include "gllc_draw_buffer.h"
#include "gllc_object.h"
#include "gllc_window_cursor.h"
#include "gllc_window_grid.h"
#include "gllc_window_selection.h"

#include <cglm/call.h>
#include <glad.h>

struct gllc_WN;
struct gllc_block;

#define GLLC_WINDOW_GRID 0x1
#define GLLC_WINDOW_SELECTION 0x2
#define GLLC_WINDOW_LPRESSED 0x4

struct gllc_window_GL
{
        GLuint GL_program;
        GLuint GL_u_MVP_loc;
        GLuint GL_u_color_loc;
        GLuint GL_u_viewport_loc;
        GLuint GL_u_scale_loc;
        GLuint GL_u_flags_loc;
        GLuint GL_u_center_point_loc;
        mat4 GL_m_proj;
        mat4 GL_m_view;
        mat4 GL_m_model;
        mat4 GL_m_MVP;
        mat4 GL_m_proj_screen;
        mat4 GL_m_MVP_screen;
};

struct gllc_window_DBG
{
        struct gllc_DBG DBG;
        struct gllc_DBG DBG_interactive;
        struct gllc_DBG DBG_screen;
};

struct gllc_window_viewport
{
        double scale_factor;
        double dx;
        double dy;
        int width;
        int height;
};

struct gllc_window_UI
{
        struct gllc_W_grid grid;
        struct gllc_W_cursor cursor;
        struct gllc_W_selection selection;
        int lpress_cursor_x;
        int lpress_cursor_y;
        int cursor_x;
        int cursor_y;
        double sel_x0;
        double sel_y0;
        double sel_x1;
        double sel_y1;
};

struct gllc_window
{
        struct gllc_object __obj;
        struct gllc_WN *native;
        struct gllc_block *block;
        struct gllc_window_UI UI;
        struct gllc_window_GL GL;
        struct gllc_window_DBG DBG;
        struct gllc_window_viewport viewport;

        GLfloat clear_color[4];

        int flags;
};

struct gllc_window *gllc_window_create(void *parent);

struct gllc_block *gllc_window_get_block(struct gllc_window *window);

void gllc_window_set_block(struct gllc_window *window, struct gllc_block *block);

void gllc_window_set_clear_color(struct gllc_window *window, int r, int g, int b);

void gllc_window_set_size(struct gllc_window *window, int x, int y, int width, int height);

void gllc_window_destroy(struct gllc_window *window);

void gllc_window_redraw(struct gllc_window *window);

void gllc_window_enable_grid(struct gllc_window *window, int enable);

void gllc_window_grid_configure(struct gllc_window *window, double gap_x, double gap_y, double offset_x, double offset_y, float *color);

void gllc_window_zoom_bb(struct gllc_window *window);

void gllc_window_wnd_to_drw(struct gllc_window *window, double x, double y, double *xd, double *yd);

#endif