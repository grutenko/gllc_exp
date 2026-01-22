#ifndef gllc_window_h
#define gllc_window_h

#include "gllc_object.h"

#include <cglm/call.h>
#include <glad.h>

struct gllc_window_native;
struct gllc_block;
struct gllc_draw_buffer;

#define GLLC_DRAW_BUFFERS 7

struct gllc_window {
  struct gllc_object __obj;
  struct gllc_window_native *native;
  struct gllc_block *block;
  GLuint u_model_loc;
  GLuint u_view_loc;
  GLuint u_projection_loc;
  GLuint u_color_loc;
  GLuint shader_program;
  mat4 m_proj;
  mat4 m_view;
  mat4 m_model;
  struct gllc_draw_buffer *draw_order[GLLC_DRAW_BUFFERS];
};

struct gllc_window *gllc_window_create(void *parent);
void gllc_window_set_block(struct gllc_window *window,
                           struct gllc_block *block);
void gllc_window_dump(const struct gllc_window *win);
void gllc_window_destroy(struct gllc_window *window);
void gllc_window_redraw(struct gllc_window *window);

#endif