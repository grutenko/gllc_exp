#ifndef gllc_draw_buffer_h
#define gllc_draw_buffer_h

#include "glad.h"

struct gllc_draw_ent_config {
  GLfloat *v;
  GLuint *i;
  GLuint v_size;
  GLuint i_size;
  GLfloat color[4];
};

struct gllc_draw_buffer;

struct gllc_draw_ent {
  struct gllc_draw_buffer *buffer;
  GLfloat *v_cache;
  GLuint *i_cache;
  GLuint v_cache_size;
  GLuint i_cache_size;
  GLuint v_cache_cap;
  GLuint i_cache_cap;
  GLuint buffer_offset;
  GLuint buffer_size;
  GLfloat color[4];
  struct gllc_draw_ent *next;
  struct gllc_draw_ent *prev;
};

struct gllc_draw_buffer {
  GLenum type;
  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  GLuint VBO_size;
  GLuint EBO_size;
  struct gllc_draw_ent *draw_ent_head;
  struct gllc_draw_ent *draw_ent_tail;
  size_t draw_ent_count;
};

struct gllc_draw {
  struct gllc_draw_buffer gl_points;
  struct gllc_draw_buffer gl_lines;
  struct gllc_draw_buffer gl_line_strip;
  struct gllc_draw_buffer gl_line_loop;
  struct gllc_draw_buffer gl_triangles;
  struct gllc_draw_buffer gl_triangle_strip;
  struct gllc_draw_buffer gl_triangle_fan;
};

void gllc_draw_init(struct gllc_draw *draw);
void gllc_draw_buffer_init(struct gllc_draw_buffer *buffer, GLenum type);

struct gllc_draw_ent *
gllc_draw_buffer_push_ent(struct gllc_draw_buffer *buffer,
                          struct gllc_draw_ent_config *ent_config);
/**
 * Удаляет все связаное с этим буфером
 */
void gllc_draw_buffer_cleanup(struct gllc_draw_buffer *buffer);

void gllc_draw_cleanup(struct gllc_draw *draw);

/**
 * Удаляет элемент, буферы не трогает чтобы привести буферы к нормальному виду
 * нужно gllc_draw_buffer_build()
 */
void gllc_draw_ent_remove(struct gllc_draw_buffer *buffer,
                          struct gllc_draw_ent *ent);
/**
 * Обновляет буфер исходя из текущего набора gllc_draw_ent
 */
int gllc_draw_buffer_build(struct gllc_draw_buffer *buffer);

int gllc_draw_build(struct gllc_draw *draw);
/**
 * Обновляет конфигурацию gllc_draw_ent
 */
int gllc_draw_ent_update(struct gllc_draw_ent *ent,
                         struct gllc_draw_ent_config *entconfig);
/**
 * Коорекно освобождет место выделеное под обьект
 */
void gllc_draw_ent_destroy(struct gllc_draw_ent *ent);
#endif