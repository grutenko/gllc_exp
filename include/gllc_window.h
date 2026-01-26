#ifndef gllc_window_h
#define gllc_window_h

#include "gllc_draw_buffer.h"
#include "gllc_object.h"

#include <cglm/call.h>
#include <glad.h>

struct gllc_WN;
struct gllc_block;

#define GLLC_DRAW_BUFFERS 14

struct gllc_window
{
        struct gllc_object __obj;
        struct gllc_WN *native;
        struct gllc_block *block;
        GLuint GL_program;
        GLuint GL_u_model_loc;
        GLuint GL_u_view_loc;
        GLuint GL_u_projection_loc;
        GLuint GL_u_color_loc;
        struct gllc_DBG_batch DBG_batch;
        struct gllc_DBG_batch DBG_batch_interactive;
        mat4 GL_m_proj;
        mat4 GL_m_view;
        mat4 GL_m_model;
        struct gllc_DBG_batch DBG_batch_screen;
        mat4 GL_m_proj_screen;
        struct gllc_DBG *DBG_order[GLLC_DRAW_BUFFERS];
        struct gllc_DBG *DBG_order_screen[7];
        double scale_factor;
        double dx;
        double dy;
        int width;
        int height;
};

struct gllc_window *gllc_window_create(void *parent);

void gllc_window_set_block(struct gllc_window *window, struct gllc_block *block);

void gllc_window_destroy(struct gllc_window *window);

void gllc_window_render(struct gllc_window *window);

#endif