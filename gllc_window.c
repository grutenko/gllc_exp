#include "gllc_window.h"
#include "glad.h"
#include "gllc_block.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"
#include "gllc_window_native.h"

#include <cglm/call.h>
#include <cglm/cglm.h>
#include <stdlib.h>

static const struct gllc_prop_def g_prop_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_prop_def, 0};

const char *vertex_shader_src =
    "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "\n"
    "uniform mat4 uModel;\n"
    "uniform mat4 uView;\n"
    "uniform mat4 uProjection;\n"
    "uniform vec4 uColor;\n"
    "\n"
    "out vec4 vColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position = uProjection * uView * uModel * vec4(aPos, 0.0, 1.0);\n"
    "    vColor = uColor;\n"
    "}\n";

const char *fragment_shader_src = "#version 330 core\n"
                                  "\n"
                                  "in vec4 vColor;\n"
                                  "out vec4 FragColor;\n"
                                  "\n"
                                  "void main()\n"
                                  "{\n"
                                  "    FragColor = vColor;\n"
                                  "}\n";

GLuint load_shader(GLuint type, const char *source) {
  GLuint shader = glCreateShader(type);
  if (shader == 0) {
    fprintf(stderr, "Failed to create shader\n");
    return 0;
  }
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled != GL_TRUE) {
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    char *log = (char *)malloc(logLength);
    if (log) {
      glGetShaderInfoLog(shader, logLength, NULL, log);
      fprintf(stderr, "Shader compilation failed:\n%s\n", log);
      free(log);
    }
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

GLuint load_program(GLuint vert_shader, GLuint frag_shader) {
  GLuint program = glCreateProgram();
  if (program == 0) {
    fprintf(stderr, "Failed to create program\n");
    return 0;
  }
  glAttachShader(program, vert_shader);
  glAttachShader(program, frag_shader);
  glLinkProgram(program);
  GLint linked = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (linked != GL_TRUE) {
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    char *log = (char *)malloc(logLength);
    if (log) {
      glGetProgramInfoLog(program, logLength, NULL, log);
      fprintf(stderr, "Program link failed:\n%s\n", log);
      free(log);
    }
    glDeleteProgram(program);
    return 0;
  }
  glDetachShader(program, vert_shader);
  glDetachShader(program, frag_shader);

  return program;
}

struct gllc_window *gllc_window_create(void *parent) {
  struct gllc_window *w = malloc(sizeof(struct gllc_window));
  if (!w)
    goto _error;
  memset(w, 0, sizeof(struct gllc_window));
  w->__obj.prop_def = g_props_def;
  w->native = gllc_WN_create(parent);
  if (!w->native)
    goto _error;

  gllc_WN_make_context_current(w->native);

  if (!gladLoadGL())
    goto _error;
  printf("Renderer: %s.\n", glGetString(GL_RENDERER));
  printf("OpenGL version supported %s.\n", glGetString(GL_VERSION));

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  GLuint vertex_shader = load_shader(GL_VERTEX_SHADER, vertex_shader_src);
  if (!vertex_shader)
    goto _error;

  GLuint fragment_shader = load_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
  if (!fragment_shader)
    goto _error;

  w->shader_program = load_program(vertex_shader, fragment_shader);
  if (!fragment_shader)
    goto _error;

  glm_mat4_identity(w->m_model);
  glm_mat4_identity(w->m_view);
  glm_mat4_identity(w->m_proj);

  int width, height;
  gllc_WN_get_size(w->native, &width, &height);
  glm_ortho(-((float)width / 2.0), (float)width / 2, -((float)height / 2.0), (float)height / 2, -1.0f, 100.0f, w->m_proj);

  w->u_model_loc = glGetUniformLocation(w->shader_program, "uModel");
  if (w->u_model_loc == -1)
    fprintf(stderr, "Uniform uModel not found!\n");
  w->u_view_loc = glGetUniformLocation(w->shader_program, "uView");
  if (w->u_view_loc == -1)
    fprintf(stderr, "Uniform uView not found!\n");
  w->u_projection_loc = glGetUniformLocation(w->shader_program, "uProjection");
  if (w->u_projection_loc == -1)
    fprintf(stderr, "Uniform uProjection not found!\n");
  GLint uColor_loc = glGetUniformLocation(w->shader_program, "uColor");
  if (uColor_loc == -1)
    fprintf(stderr, "Uniform uColor not found!\n");

  gllc_WN_make_context_current(0);

  return w;
_error:
  if (w) {
    if (w->native) {
      gllc_WN_destroy(w->native);
    }
    free(w);
  }
  return 0;
}

void gllc_window_dump(const struct gllc_window *win) {
  if (!win) {
    printf("gllc_window: NULL\n");
    return;
  }
  printf("gllc_window @ %p\n", (void *)win);
  printf("  __obj              = %p\n", (void *)&win->__obj);
  printf("  native             = %p\n", (void *)win->native);
  printf("  block              = %p\n", (void *)win->block);
  printf("  shader_program     = %u\n", win->shader_program);
  printf("  u_model_loc        = %u\n", win->u_model_loc);
  printf("  u_view_loc         = %u\n", win->u_view_loc);
  printf("  u_projection_loc   = %u\n", win->u_projection_loc);
  printf("  u_color_loc        = %u\n", win->u_color_loc);
  printf("  m_proj:\n");
  for (int i = 0; i < 4; ++i) {
    printf("    [ %f %f %f %f ]\n", win->m_proj[i][0], win->m_proj[i][1], win->m_proj[i][2], win->m_proj[i][3]);
  }
  printf("  m_view:\n");
  for (int i = 0; i < 4; ++i) {
    printf("    [ %f %f %f %f ]\n", win->m_view[i][0], win->m_view[i][1], win->m_view[i][2], win->m_view[i][3]);
  }
  printf("  m_model:\n");
  for (int i = 0; i < 4; ++i) {
    printf("    [ %f %f %f %f ]\n", win->m_model[i][0], win->m_model[i][1], win->m_model[i][2], win->m_model[i][3]);
  }
}

void gllc_window_set_block(struct gllc_window *window,
                           struct gllc_block *block) {
  window->block = block;
  window->draw_order[0] = &window->block->draw_batch.gl_triangles;
  window->draw_order[1] = &window->block->draw_batch.gl_triangle_strip;
  window->draw_order[2] = &window->block->draw_batch.gl_triangle_fan;
  window->draw_order[3] = &window->block->draw_batch.gl_lines;
  window->draw_order[4] = &window->block->draw_batch.gl_line_strip;
  window->draw_order[5] = &window->block->draw_batch.gl_line_loop;
  window->draw_order[6] = &window->block->draw_batch.gl_points;
}
void gllc_window_destroy(struct gllc_window *window) {
  gllc_WN_destroy(window->native);
  free(window);
}

void gllc_window_redraw(struct gllc_window *window) {
  glClear(GL_COLOR_BUFFER_BIT);
  if (window->block) {
    glUseProgram(window->shader_program);
    glUniformMatrix4fv(window->u_model_loc, 1, GL_FALSE, (const float *)window->m_model);
    glUniformMatrix4fv(window->u_view_loc, 1, GL_FALSE, (const float *)window->m_view);
    glUniformMatrix4fv(window->u_projection_loc, 1, GL_FALSE, (const float *)window->m_proj);
    for (int i = 0; i < GLLC_DRAW_BUFFERS; i++) {
      glBindVertexArray(window->draw_order[i]->VAO);
      struct gllc_draw_ent *ent = window->draw_order[i]->draw_ent_head;
      while (ent) {
        glUniform4f(window->u_color_loc, ent->color[0], ent->color[1], ent->color[2], ent->color[3]);
        glDrawElements(window->draw_order[i]->type, ent->i_cache_size, GL_UNSIGNED_INT, (void *)(sizeof(GLuint) * ent->buffer_offset));
        ent = ent->next;
      }
    }
  }
  gllc_WN_swap_buffers(window->native);
}