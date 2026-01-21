#include "gllc_draw_buffer.h"
#include "glad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void gllc_draw_init(struct gllc_draw *draw) {
  gllc_draw_buffer_init(&draw->gl_line_loop, GL_LINE_LOOP);
  gllc_draw_buffer_init(&draw->gl_line_strip, GL_LINE_STRIP);
  gllc_draw_buffer_init(&draw->gl_lines, GL_LINES);
  gllc_draw_buffer_init(&draw->gl_triangle_strip, GL_LINES);
  gllc_draw_buffer_init(&draw->gl_triangle_fan, GL_TRIANGLE_FAN);
  gllc_draw_buffer_init(&draw->gl_triangles, GL_TRIANGLES);
  gllc_draw_buffer_init(&draw->gl_points, GL_POINTS);
}

void gllc_draw_buffer_init(struct gllc_draw_buffer *buffer, GLenum type) {
  buffer->type = type;
  glGenVertexArrays(1, &buffer->VAO);
  glBindVertexArray(buffer->VAO);
  glGenBuffers(1, &buffer->VBO);
  glGenBuffers(1, &buffer->EBO);
  glBindVertexArray(0);
  buffer->VBO_size = 0;
  buffer->EBO_size = 0;
  buffer->draw_ent_count = 0;
  buffer->draw_ent_head = 0;
  buffer->draw_ent_tail = 0;
}

void gllc_draw_buffer_cleanup(struct gllc_draw_buffer *buffer) {
  if (!buffer)
    return;
  struct gllc_draw_ent *ent = buffer->draw_ent_head;
  while (ent) {
    struct gllc_draw_ent *next = ent->next;
    free(ent->v_cache);
    free(ent->i_cache);
    free(ent);
    ent = next;
  }
  buffer->draw_ent_head = NULL;
  buffer->draw_ent_tail = NULL;
  buffer->draw_ent_count = 0;
  if (buffer->VBO)
    glDeleteBuffers(1, &buffer->VBO);
  if (buffer->EBO)
    glDeleteBuffers(1, &buffer->EBO);
  if (buffer->VAO)
    glDeleteVertexArrays(1, &buffer->VAO);
  buffer->VBO = 0;
  buffer->EBO = 0;
  buffer->VAO = 0;
  buffer->VBO_size = 0;
  buffer->EBO_size = 0;
}

void gllc_draw_cleanup(struct gllc_draw *draw) {
  if (!draw)
    return;
  gllc_draw_buffer_cleanup(&draw->gl_points);
  gllc_draw_buffer_cleanup(&draw->gl_lines);
  gllc_draw_buffer_cleanup(&draw->gl_line_strip);
  gllc_draw_buffer_cleanup(&draw->gl_line_loop);
  gllc_draw_buffer_cleanup(&draw->gl_triangles);
  gllc_draw_buffer_cleanup(&draw->gl_triangle_strip);
  gllc_draw_buffer_cleanup(&draw->gl_triangle_fan);
}

int gllc_draw_buffer_build(struct gllc_draw_buffer *buffer) {
  GLuint total_v = 0;
  GLuint total_i = 0;
  struct gllc_draw_ent *ent = buffer->draw_ent_head;
  while (ent) {
    total_v += ent->v_cache_size;
    total_i += ent->i_cache_size;
    ent = ent->next;
  }
  buffer->VBO_size = total_v;
  buffer->EBO_size = total_i;
  if (total_v == 0 || total_i == 0)
    return 1;
  GLfloat *v_all = malloc(sizeof(GLfloat) * total_v);
  GLuint *i_all = malloc(sizeof(GLuint) * total_i);
  if (!v_all || !i_all) {
    free(v_all);
    free(i_all);
    return 0;
  }
  GLuint v_offset = 0;
  GLuint i_offset = 0;
  ent = buffer->draw_ent_head;
  while (ent) {
    memcpy(v_all + v_offset, ent->v_cache, sizeof(GLfloat) * ent->v_cache_size);
    for (GLuint k = 0; k < ent->i_cache_size; k++) {
      i_all[i_offset + k] =
          ent->i_cache[k] + v_offset / 2; // предполагаем 2 float на вершину
    }
    ent->buffer_size = ent->i_cache_size;
    ent->buffer_offset = i_offset;
    v_offset += ent->v_cache_size;
    i_offset += ent->i_cache_size;
    ent = ent->next;
  }
  glBindVertexArray(buffer->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, buffer->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * total_v, v_all,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * total_i, i_all,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                        (void *)0);
  glBindVertexArray(0);
  free(v_all);
  free(i_all);
  printf("Draw buffer build %d vertixes.\n", total_i);
  return 1;
}

int gllc_draw_build(struct gllc_draw *draw) {
  int ok = 1;
  ok &= gllc_draw_buffer_build(&draw->gl_points);
  ok &= gllc_draw_buffer_build(&draw->gl_lines);
  ok &= gllc_draw_buffer_build(&draw->gl_line_strip);
  ok &= gllc_draw_buffer_build(&draw->gl_line_loop);
  ok &= gllc_draw_buffer_build(&draw->gl_triangles);
  ok &= gllc_draw_buffer_build(&draw->gl_triangle_strip);
  ok &= gllc_draw_buffer_build(&draw->gl_triangle_fan);
  return ok;
}

struct gllc_draw_ent *
gllc_draw_buffer_push_ent(struct gllc_draw_buffer *buffer,
                          struct gllc_draw_ent_config *ent_config) {
  if (!buffer || !ent_config)
    return NULL;
  struct gllc_draw_ent *ent =
      (struct gllc_draw_ent *)malloc(sizeof(struct gllc_draw_ent));
  if (!ent)
    return NULL;
  memset(ent, 0, sizeof(struct gllc_draw_ent));
  ent->buffer = buffer;
  ent->v_cache_cap = ent_config->v_size;
  ent->v_cache_size = ent_config->v_size;
  ent->v_cache = (GLfloat *)malloc(sizeof(GLfloat) * ent->v_cache_cap);
  if (!ent->v_cache) {
    free(ent);
    return NULL;
  }
  memcpy(ent->v_cache, ent_config->v, sizeof(GLfloat) * ent_config->v_size);
  if (ent_config->i_size > 0 && ent_config->i) {
    ent->i_cache_cap = ent_config->i_size;
    ent->i_cache_size = ent_config->i_size;
    ent->i_cache = (GLuint *)malloc(sizeof(GLuint) * ent->i_cache_cap);
    if (!ent->i_cache) {
      free(ent->v_cache);
      free(ent);
      return NULL;
    }
    memcpy(ent->i_cache, ent_config->i, sizeof(GLuint) * ent_config->i_size);
  }
  memcpy(ent->color, ent_config->color, sizeof(GLfloat) * 4);
  ent->prev = buffer->draw_ent_tail;
  ent->next = NULL;
  if (buffer->draw_ent_tail)
    buffer->draw_ent_tail->next = ent;
  else
    buffer->draw_ent_head = ent;
  buffer->draw_ent_tail = ent;
  buffer->draw_ent_count++;

  return ent;
}

int gllc_draw_ent_update(struct gllc_draw_ent *ent,
                         struct gllc_draw_ent_config *entconfig) {
  if (!ent || !entconfig)
    return 0;
  if (ent->buffer == NULL)
    return 0;
  if (entconfig->v_size > 0 && entconfig->v) {
    if (entconfig->v_size > ent->v_cache_cap) {
      GLfloat *new_v =
          (GLfloat *)realloc(ent->v_cache, sizeof(GLfloat) * entconfig->v_size);
      if (!new_v)
        return 0;
      ent->v_cache = new_v;
      ent->v_cache_cap = entconfig->v_size;
    }
    memcpy(ent->v_cache, entconfig->v, sizeof(GLfloat) * entconfig->v_size);
    ent->v_cache_size = entconfig->v_size;
  }
  if (entconfig->i_size > 0 && entconfig->i) {
    if (entconfig->i_size > ent->i_cache_cap) {
      GLuint *new_i =
          (GLuint *)realloc(ent->i_cache, sizeof(GLuint) * entconfig->i_size);
      if (!new_i)
        return 0;
      ent->i_cache = new_i;
      ent->i_cache_cap = entconfig->i_size;
    }
    memcpy(ent->i_cache, entconfig->i, sizeof(GLuint) * entconfig->i_size);
    ent->i_cache_size = entconfig->i_size;
  } else {
    ent->i_cache_size = 0;
  }
  memcpy(ent->color, entconfig->color, sizeof(GLfloat) * 4);
  return 1;
}

void gllc_draw_ent_remove(struct gllc_draw_buffer *buffer,
                          struct gllc_draw_ent *ent) {
  if (!buffer || !ent)
    return;
  if (ent->buffer != buffer)
    return;
  if (ent->prev)
    ent->prev->next = ent->next;
  else
    buffer->draw_ent_head = ent->next;
  if (ent->next)
    ent->next->prev = ent->prev;
  else
    buffer->draw_ent_tail = ent->prev;
  buffer->draw_ent_count--;
}

void gllc_draw_ent_destroy(struct gllc_draw_ent *ent) {
  free(ent->i_cache);
  free(ent->v_cache);
  free(ent);
}