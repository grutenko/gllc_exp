#include "cglm/types.h"
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

struct EntityConfig {
  GLenum type;
  GLfloat *v;
  GLuint *i;
  GLint v_size;
  GLint i_size;
  GLfloat color[4];
};

struct Entity {
  GLenum type;
  GLuint count;
  float color[4];
  GLuint _startIndex;
};

#define INITIAL_CAPACITY 64

struct EntityArray {
  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  GLuint VBO_size;
  GLuint EBO_size;
  struct Entity *data; // массив указателей на сущности
  size_t count;        // текущее количество
  size_t capacity;     // текущая вместимость
};

#define MAX_VBO_SIZE (12 * 1024 * 1024) // 12 МБ для VBO
#define MAX_EBO_SIZE (12 * 1024 * 1024) // 12 МБ для EBO

static struct EntityArray G_fill = {0};
static struct EntityArray G_line = {0};
static struct Entity *push_entity(struct EntityArray *arr,
                                  const struct EntityConfig *ent_config) {
  // Расширяем массив сущностей на CPU
  if (arr->count >= arr->capacity) {
    size_t new_capacity = arr->capacity ? arr->capacity * 2 : INITIAL_CAPACITY;
    struct Entity *new_data =
        realloc(arr->data, new_capacity * sizeof(struct Entity));
    if (!new_data) {
      fprintf(stderr, "Out of memory!\n");
      exit(1);
    }
    arr->data = new_data;
    arr->capacity = new_capacity;
  }

  struct Entity *e = &arr->data[arr->count];
  e->type = ent_config->type;
  e->count = ent_config->i_size;
  memcpy(e->color, ent_config->color, sizeof(float) * 4);
  e->_startIndex = arr->EBO_size; // смещение в EBO для draw call

  // Создаём VAO один раз
  if (!arr->VAO)
    glGenVertexArrays(1, &arr->VAO);
  glBindVertexArray(arr->VAO);

  // Создаём VBO один раз
  if (!arr->VBO) {
    glGenBuffers(1, &arr->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, arr->VBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_VBO_SIZE, NULL,
                 GL_DYNAMIC_DRAW); // заранее резервируем память
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
  } else {
    glBindBuffer(GL_ARRAY_BUFFER, arr->VBO);
  }

  // Добавляем новые вершины в конец буфера
  GLuint vertexOffset = arr->VBO_size / 2; // если 2D (x,y)
  glBufferSubData(GL_ARRAY_BUFFER, arr->VBO_size * sizeof(float),
                  ent_config->v_size * sizeof(float), ent_config->v);
  arr->VBO_size += ent_config->v_size;

  // Создаём EBO один раз
  if (!arr->EBO) {
    glGenBuffers(1, &arr->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arr->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_EBO_SIZE, NULL, GL_DYNAMIC_DRAW);
  } else {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arr->EBO);
  }

  // Загружаем индексы с нужным смещением
  GLuint *shifted_indices = malloc(ent_config->i_size * sizeof(GLuint));
  for (int j = 0; j < ent_config->i_size; j++)
    shifted_indices[j] = ent_config->i[j] + vertexOffset;

  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, arr->EBO_size * sizeof(GLuint),
                  ent_config->i_size * sizeof(GLuint), shifted_indices);
  free(shifted_indices);

  arr->EBO_size += ent_config->i_size;
  arr->count++;

  glBindVertexArray(0);
  return e;
}

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
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
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
  int x;
  int y;
  float v[9] = {-100.0f, -100.0f, 100.0f,  -100.0f,
                100.0f,  100.0f,  -100.0f, 100.0f};
  unsigned int indx[] = {0, 1, 2, 3, 0, 0};
  for (x = -200; x < 200; x += 2) {
    for (y = -200; y < 200; y += 2) {
      struct EntityConfig entConf;
      entConf.type = GL_TRIANGLES;
      v[0] = x;
      v[1] = y;
      v[2] = x + 2;
      v[3] = y;
      v[4] = x + 2;
      v[5] = y + 2;
      v[6] = x;
      v[7] = y + 2;
      indx[0] = 0;
      indx[1] = 1;
      indx[2] = 2;
      indx[3] = 0;
      indx[4] = 2;
      indx[5] = 3;

      // Генерация случайного цвета
      entConf.color[0] = (float)rand() / RAND_MAX; // R
      entConf.color[1] = (float)rand() / RAND_MAX; // G
      entConf.color[2] = (float)rand() / RAND_MAX; // B
      entConf.color[3] = 1.0f; // A всегда 1 (полностью непрозрачный)
      entConf.v = v;
      entConf.i = indx;
      entConf.v_size = 8;
      entConf.i_size = 6;
      push_entity(&G_fill, &entConf);

      entConf.type = GL_LINE_LOOP;
      indx[0] = 0;
      indx[1] = 1;
      indx[2] = 2;
      indx[3] = 3;
      entConf.color[0] = 0.0;
      entConf.color[1] = 0.0;
      entConf.color[2] = 0.0f;
      entConf.color[3] = 1.0f;
      entConf.v = v;
      entConf.i = indx;
      entConf.v_size = 8;
      entConf.i_size = 4;
      push_entity(&G_line, &entConf);
    }
  }
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
    glBindVertexArray(G_fill.VAO);
    for (size_t i = 0; i < G_fill.count; i++) {
      glUniform4f(uColor_loc, G_fill.data[i].color[0], G_fill.data[i].color[1],
                  G_fill.data[i].color[2], G_fill.data[i].color[3]);
      glDrawElements(G_fill.data[i].type, G_fill.data[i].count, GL_UNSIGNED_INT,
                     (void *)(G_fill.data[i]._startIndex * sizeof(GLuint)));
    }
    glBindVertexArray(G_line.VAO);
    for (size_t i = 0; i < G_line.count; i++) {
      glUniform4f(uColor_loc, G_line.data[i].color[0], G_line.data[i].color[1],
                  G_line.data[i].color[2], G_line.data[i].color[3]);
      glDrawElements(G_line.data[i].type, G_line.data[i].count, GL_UNSIGNED_INT,
                     (void *)(G_line.data[i]._startIndex * sizeof(GLuint)));
    }
    glfwSwapBuffers(window);
    glfwWaitEvents();
    check_gl_error("GLerror");
  }
  glDeleteVertexArrays(1, &G_fill.VAO);
  glDeleteBuffers(1, &G_fill.VBO);
  glDeleteBuffers(1, &G_fill.EBO);
  glDeleteVertexArrays(1, &G_line.VAO);
  glDeleteBuffers(1, &G_line.VBO);
  glDeleteBuffers(1, &G_line.EBO);
  free(G_fill.data);
  free(G_line.data);
  glfwTerminate();
  return 0;
_error:
  if (window)
    glfwDestroyWindow(window);
  glfwTerminate();
  return -1;
}