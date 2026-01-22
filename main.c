#include "gllc_block.h"
#include "gllc_block_entity.h"
#include "gllc_draw_buffer.h"
#include "gllc_drawing.h"
#include "gllc_polyline.h"
#include <stdlib.h>

#define GLFW_INCLUDE_NONE
#include "glad.h"
#include <GLFW/glfw3.h>
#include <cglm/call.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h> // для rand()
#include <time.h>   // для srand()

#include <windows.h>

GLFWwindow *window = NULL;

GLint G_shader_program = 0;

mat4 G_mproj, G_mview, G_model;

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

const char *gl_error_string(GLenum err) {
  switch (err) {
  case GL_NO_ERROR:
    return "No error";
  case GL_INVALID_ENUM:
    return "Invalid enum";
  case GL_INVALID_VALUE:
    return "Invalid value";
  case GL_INVALID_OPERATION:
    return "Invalid operation";
  case GL_STACK_OVERFLOW:
    return "Stack overflow";
  case GL_STACK_UNDERFLOW:
    return "Stack underflow";
  case GL_OUT_OF_MEMORY:
    return "Out of memory";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "Invalid framebuffer operation";
  default:
    return "Unknown error";
  }
}

void check_gl_error(const char *msg) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "OpenGL error [%s]: 0x%X (%s)\n", msg, err,
            gl_error_string(err));
  }
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

int G_left_pressed = 0;
double G_last_xpos = 0.0f;
double G_last_ypos = 0.0f;
double G_scale = 1.0;

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      glfwGetCursorPos(window, &G_last_xpos, &G_last_ypos);
    }
  }
  G_left_pressed = (action == GLFW_PRESS);
}

static void cursor_position_callback(GLFWwindow *window, double xpos,
                                     double ypos) {
  if (G_left_pressed) {
    double dx = (G_last_xpos - xpos) * G_scale;
    double dy = (G_last_ypos - ypos) * G_scale;
    vec3 translation = {-dx, dy, 0.0f};
    glm_translate(G_mview, translation);
    G_last_xpos = xpos;
    G_last_ypos = ypos;
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  width *= G_scale;
  height *= G_scale;
  glm_ortho(-((float)width / 2.0), (float)width / 2, -((float)height / 2.0),
            (float)height / 2, -1.0f, 100.0f, G_mproj);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  G_scale *= 1 + (yoffset / 10);
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  width *= G_scale;
  height *= G_scale;
  glm_ortho(-((float)width / 2.0), (float)width / 2, -((float)height / 2.0),
            (float)height / 2, -1.0f, 100.0f, G_mproj);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline,
                     int cmdshow) {

  srand((unsigned int)time(NULL));
  if (!glfwInit())
    return -1;
  glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
  if (!window)
    goto _error;
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  if (!gladLoadGL())
    goto _error;
  printf("Renderer: %s.\n", glGetString(GL_RENDERER));
  printf("OpenGL version supported %s.\n", glGetString(GL_VERSION));
  // glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  GLuint vertex_shader = load_shader(GL_VERTEX_SHADER, vertex_shader_src);
  if (!vertex_shader)
    goto _error;
  GLuint fragment_shader = load_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
  if (!fragment_shader)
    goto _error;
  G_shader_program = load_program(vertex_shader, fragment_shader);
  if (!fragment_shader)
    goto _error;
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glm_mat4_identity(G_model);
  glm_mat4_identity(G_mview);
  glm_mat4_identity(G_mproj);
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  glm_ortho(-((float)width / 2.0), (float)width / 2, -((float)height / 2.0),
            (float)height / 2, -1.0f, 100.0f, G_mproj);
  GLint uModel_loc = glGetUniformLocation(G_shader_program, "uModel");
  if (uModel_loc == -1)
    fprintf(stderr, "Uniform uModel not found!\n");
  GLint uView_loc = glGetUniformLocation(G_shader_program, "uView");
  if (uView_loc == -1)
    fprintf(stderr, "Uniform uView not found!\n");
  GLint uProjection_loc = glGetUniformLocation(G_shader_program, "uProjection");
  if (uProjection_loc == -1)
    fprintf(stderr, "Uniform uProjection not found!\n");
  GLint uColor_loc = glGetUniformLocation(G_shader_program, "uColor");
  if (uColor_loc == -1)
    fprintf(stderr, "Uniform uColor not found!\n");

  struct gllc_drawing *hDrw = gllc_drawing_create();
  struct gllc_block *hBlock = gllc_drw_add_block(hDrw, "ModelSpace", 0.0, 0.0);
  struct gllc_polyline *hPline = gllc_block_add_polyline(hBlock, 1);
  gllc_polyline_add_ver(hPline, 0.0f, 300.0f);     // вершина сверху
  gllc_polyline_add_ver(hPline, 58.78f, 95.11f);   // правая верхняя
  gllc_polyline_add_ver(hPline, 293.89f, 95.11f);  // правая
  gllc_polyline_add_ver(hPline, 95.11f, -36.60f);  // правая нижняя
  gllc_polyline_add_ver(hPline, 150.0f, -240.0f);  // низ справа
  gllc_polyline_add_ver(hPline, 0.0f, -95.0f);     // низ
  gllc_polyline_add_ver(hPline, -150.0f, -240.0f); // низ слева
  gllc_polyline_add_ver(hPline, -95.11f, -36.60f); // левая нижняя
  gllc_polyline_add_ver(hPline, -293.89f, 95.11f); // левая
  gllc_polyline_add_ver(hPline, -58.78f, 95.11f);  // верхняя левая
  gllc_block_update(hBlock);

  struct gllc_draw_buffer *draw_order[] = {
      &hBlock->draw.gl_triangles,    &hBlock->draw.gl_triangle_strip,
      &hBlock->draw.gl_triangle_fan, &hBlock->draw.gl_lines,
      &hBlock->draw.gl_line_strip,   &hBlock->draw.gl_line_loop,
      &hBlock->draw.gl_points,
  };

  double lastTime = glfwGetTime();
  int frameCount = 0;
  while (!glfwWindowShouldClose(window)) {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime;
    frameCount++;
    if (deltaTime >= 1.0) {
      double fps = frameCount / deltaTime;
      printf("FPS: %lf, Frame time: %lf ms\n", fps, 1000.0 / fps);
      frameCount = 0;
      lastTime = currentTime;
    }
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(G_shader_program);
    glUniformMatrix4fv(uModel_loc, 1, GL_FALSE, (const float *)G_model);
    glUniformMatrix4fv(uView_loc, 1, GL_FALSE, (const float *)G_mview);
    glUniformMatrix4fv(uProjection_loc, 1, GL_FALSE, (const float *)G_mproj);
    int i;
    for (i = 0; i < sizeof(draw_order) / sizeof(struct gllc_draw_buffer *);
         i++) {
      glBindVertexArray(draw_order[i]->VAO);
      struct gllc_draw_ent *ent = draw_order[i]->draw_ent_head;
      while (ent) {
        glUniform4f(uColor_loc, ent->color[0], ent->color[1], ent->color[2],
                    ent->color[3]);
        glDrawElements(draw_order[i]->type, ent->i_cache_size, GL_UNSIGNED_INT,
                       (void *)(sizeof(GLuint) * ent->buffer_offset));
        ent = ent->next;
      }
    }
    glfwSwapBuffers(window);
    glfwWaitEvents();
  }
  glfwTerminate();
  return 0;
_error:
  if (window)
    glfwDestroyWindow(window);
  glfwTerminate();
  return -1;
}