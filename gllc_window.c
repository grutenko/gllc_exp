#include "gllc_window.h"
#include "glad.h"
#include "gllc_block.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"
#include "gllc_window_cursor.h"
#include "gllc_window_grid.h"
#include "gllc_window_native.h"

#include "gllc_fragment_shader.h"
#include "gllc_vertex_shader.h"
#include "gllc_window_selection.h"
#include "include/gllc_window_grid.h"

#include <cglm/call.h>
#include <cglm/cglm.h>
#include <stdlib.h>

static const struct gllc_prop_def g_prop_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_prop_def, 0};

static GLuint load_shader(GLuint type, const char *source)
{
        GLuint shader = glCreateShader(type);
        if (shader == 0)
        {
                fprintf(stderr, "Failed to create shader\n");
                return 0;
        }
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (compiled != GL_TRUE)
        {
                GLint logLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
                char *log = (char *)malloc(logLength);
                if (log)
                {
                        glGetShaderInfoLog(shader, logLength, NULL, log);
                        fprintf(stderr, "Shader compilation failed:\n%s\n", log);
                        free(log);
                }
                glDeleteShader(shader);
                return 0;
        }
        return shader;
}

static GLuint load_program(GLuint vert_shader, GLuint frag_shader)
{
        GLuint program = glCreateProgram();
        if (program == 0)
        {
                fprintf(stderr, "Failed to create program\n");
                return 0;
        }
        glAttachShader(program, vert_shader);
        glAttachShader(program, frag_shader);
        glLinkProgram(program);
        GLint linked = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE)
        {
                GLint logLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
                char *log = (char *)malloc(logLength);
                if (log)
                {
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

static GLuint load_GL_program()
{
        GLuint V_shader = load_shader(GL_VERTEX_SHADER, gllc_vertex_shader_vert);
        if (!V_shader)
        {
                return 0;
        }
        GLuint F_shader = load_shader(GL_FRAGMENT_SHADER, gllc_fragment_shader_frag);
        if (!F_shader)
        {
                return 0;
        }
        return load_program(V_shader, F_shader);
}

static inline void render_camera(struct gllc_window *w)
{
        float half_w = (float)w->width / 2.0f * w->scale_factor;
        float half_h = (float)w->height / 2.0f * w->scale_factor;

        glm_ortho(-half_w, half_w, -half_h, half_h, -1000.0f, 1000.0f, w->GL_m_proj);

        glm_ortho(0.0f, (float)w->width, (float)w->height, 0.0f, -1.0f, 1.0f, w->GL_m_proj_screen);

        glm_mat4_identity(w->GL_m_view);
        vec4 t = {w->dx, w->dy, 0.0f, 1.0f};
        glm_translate(w->GL_m_view, t);

        glm_mat4_mul(w->GL_m_proj, w->GL_m_view, w->GL_m_MVP);
        glm_mat4_mul(w->GL_m_MVP, w->GL_m_model, w->GL_m_MVP);

        glm_mat4_copy(w->GL_m_proj_screen, w->GL_m_MVP_screen);
}

static void update_viewport(struct gllc_window *w)
{
        render_camera(w);

        struct gllc_W_grid_viewport v;
        gllc_window_wnd_to_drw(w, 0.0f, 0.0f, &v.x0, &v.y0);
        gllc_window_wnd_to_drw(w, (double)w->width, (double)w->height, &v.x1, &v.y1);
        v.scale = w->scale_factor;

        struct gllc_W_grid_config conf = {
            .clear_color = NULL,
            .color = NULL,
            .gap = NULL,
            .offset = NULL,
            .viewport = &v};

        gllc_W_grid_configure(&w->grid, &conf);
}

static void load_GL_uniform_loc(struct gllc_window *w)
{
#define LOADLOC(out, var)                                        \
        out = glGetUniformLocation(w->GL_program, var);          \
        if (out == -1)                                           \
        {                                                        \
                fprintf(stderr, "Uniform %s not found!\n", var); \
        }

        LOADLOC(w->GL_u_MVP_loc, "uMVP");
        LOADLOC(w->GL_u_color_loc, "uColor");
        LOADLOC(w->GL_u_viewport_loc, "uViewport");
        LOADLOC(w->GL_u_scale_loc, "uScale");
        LOADLOC(w->GL_u_flags_loc, "uFlags");
        LOADLOC(w->GL_u_center_point_loc, "uCenterPoint");
}

struct drag_state
{
        int dragging;
        int last_x;
        int last_y;
};

static struct drag_state g_drag = {0};

static void on_mouse_click(struct gllc_WN *wn, int x, int y, int mode, int action, void *USER_1)
{
        struct gllc_window *w = (struct gllc_window *)USER_1;

        if (mode == 3)
        {
                if (action == 1)
                {
                        g_drag.dragging = 1;
                        g_drag.last_x = x;
                        g_drag.last_y = y;
                }
                else if (action == 0)
                {
                        g_drag.dragging = 0;
                }
        }
        else if (mode == 1)
        {
                if (action == 1)
                {
                        w->in_selection = 1;
                        w->sel_x0 = (double)(x - (int)(w->width / 2)) * w->scale_factor - w->dx;
                        w->sel_y0 = (double)((w->height - y) - (int)(w->height / 2)) * w->scale_factor - w->dy;
                        w->sel_x1 = w->sel_x0 + 1.0f * w->scale_factor;
                        w->sel_y1 = w->sel_y0 + 1.0f * w->scale_factor;
                }
                else if (action == 0)
                {
                        w->in_selection = 0;
                }
        }

        gllc_WN_dirty(wn);
}

static void draw(struct gllc_window *w);

static LARGE_INTEGER g_freq;
static LARGE_INTEGER g_last;
static int g_frame_count = 0;

void fps_init()
{
        QueryPerformanceFrequency(&g_freq);
        QueryPerformanceCounter(&g_last);
}

static void fps_tick()
{
        g_frame_count++;

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);

        double elapsed = (double)(now.QuadPart - g_last.QuadPart) / g_freq.QuadPart;

        if (elapsed >= 1.0)
        {
                double fps = g_frame_count / elapsed;
                printf("FPS(mousemove): %.2f\n", fps);

                g_frame_count = 0;
                g_last = now;
        }
}

static void on_mouse_move(struct gllc_WN *wn, int x, int y, void *USER_1)
{
        struct gllc_window *w = (struct gllc_window *)USER_1;

        fps_tick();

        if (g_drag.dragging)
        {
                int dx = x - g_drag.last_x;
                int dy = y - g_drag.last_y;
                g_drag.last_x = x;
                g_drag.last_y = y;
                w->dx += dx * w->scale_factor;
                w->dy -= dy * w->scale_factor;

                update_viewport(w);
        }

        if (w->in_selection)
        {
                w->sel_x1 = (x - (int)(w->width / 2)) * w->scale_factor - w->dx;
                w->sel_y1 = ((w->height - y) - (int)(w->height / 2)) * w->scale_factor - w->dy;
        }

        w->cursor_x = x;
        w->cursor_y = y;

        gllc_WN_dirty(w->native);
}

static void on_mouse_scroll(struct gllc_WN *wn, int dx, int dy, void *USER_1)
{
        struct gllc_window *w = (struct gllc_window *)USER_1;

        if (dy >= 10)
                dy = 2;
        if (dy <= -10)
                dy = -2;

        if ((w->scale_factor > 300.0f && dy > 0) || (w->scale_factor < 0.005f && dy < 0))
                return;

        double old_scale = w->scale_factor;
        w->scale_factor *= 1.0f + ((double)dy * 0.1);

        int mouse_x, mouse_y;
        gllc_WN_get_cursor(wn, &mouse_x, &mouse_y);

        w->dx += (mouse_x - (int)(w->width / 2)) * (w->scale_factor - old_scale);
        w->dy += ((w->height - mouse_y) - (int)(w->height / 2)) * (w->scale_factor - old_scale);

        update_viewport(w);

        gllc_WN_dirty(w->native);
}

static size_t draw_DBG(GLuint color_loc, GLuint flags_loc, GLuint center_point_loc, struct gllc_DBG *DBG, GLfloat wx0, GLfloat wy0, GLfloat wx1, GLfloat wy1)
{
        int i;
        for (i = 0; i < DBG->DE_size; i++)
        {
                if (!(DBG->DE[i].BBox_x1 > wx0 && DBG->DE[i].BBox_y1 > wy0 && DBG->DE[i].BBox_x0 < wx1 && DBG->DE[i].BBox_y0 < wy1))
                {
                         continue;
                }

                glUniform1ui(flags_loc, DBG->DE[i].flags);
                glUniform2f(center_point_loc, DBG->DE[i].center_point[0], DBG->DE[i].center_point[1]);
                glUniform4f(color_loc,
                            DBG->DE[i].color[0],
                            DBG->DE[i].color[1],
                            DBG->DE[i].color[2],
                            DBG->DE[i].color[3]);

                glDrawElements(DBG->DE[i].GL_type, DBG->DE[i].size, GL_UNSIGNED_INT, (void *)(sizeof(GLuint) * DBG->DE[i].offset));
        }

        glUniform1ui(flags_loc, 0);

        return DBG->DE_size;
}

void CheckOpenGLError(const char *context)
{
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
                const char *err_str = "UNKNOWN";
                switch (err)
                {
                case GL_INVALID_ENUM:
                        err_str = "GL_INVALID_ENUM";
                        break;
                case GL_INVALID_VALUE:
                        err_str = "GL_INVALID_VALUE";
                        break;
                case GL_INVALID_OPERATION:
                        err_str = "GL_INVALID_OPERATION";
                        break;
                case GL_STACK_OVERFLOW:
                        err_str = "GL_STACK_OVERFLOW";
                        break;
                case GL_STACK_UNDERFLOW:
                        err_str = "GL_STACK_UNDERFLOW";
                        break;
                case GL_OUT_OF_MEMORY:
                        err_str = "GL_OUT_OF_MEMORY";
                        break;
                case GL_INVALID_FRAMEBUFFER_OPERATION:
                        err_str = "GL_INVALID_FRAMEBUFFER_OPERATION";
                        break;
                }
                printf("OpenGL Error [%s]: %s (0x%X)\n", context, err_str, err);
        }
}

static void draw(struct gllc_window *w)
{
        gllc_WN_make_context_current(w->native);

        if (w->block)
        {
                gllc_DBG_build(&w->DBG, &w->block->DBD);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(w->GL_program);
        glUniform1ui(w->GL_u_flags_loc, 0);
        glUniformMatrix4fv(w->GL_u_MVP_loc, 1, GL_FALSE, (const float *)w->GL_m_MVP);

        GLfloat viewport[2] = {(GLfloat)w->width, (GLfloat)w->height};
        glUniform2fv(w->GL_u_viewport_loc, 1, (const float *)viewport);
        glUniform1f(w->GL_u_scale_loc, w->scale_factor);

        if (w->grid_used)
        {
                gllc_W_grid_draw(&w->grid, w->GL_u_color_loc);
        }

        glBindVertexArray(w->DBG.VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, w->DBG.EBO);

        double wx0, wy0, wx1, wy1;
        gllc_window_wnd_to_drw(w, 0.0f, 0.0f, &wx0, &wy0);
        gllc_window_wnd_to_drw(w, (double)w->width, (double)w->height, &wx1, &wy1);

        draw_DBG(w->GL_u_color_loc, w->GL_u_flags_loc, w->GL_u_center_point_loc, &w->DBG, wx0, wy0, wx1, wy1);

        /*glBindVertexArray(w->DBG_interactive.VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, w->DBG_interactive.EBO);
        draw_DBG(w->GL_u_color_loc, w->GL_u_flags_loc, w->GL_u_center_point_loc, &w->DBG_interactive, wx0, wy0, wx1, wy1);*/

        if (w->in_selection)
        {
                gllc_W_selection_draw(&w->selection, w->GL_u_color_loc, w->sel_x0, w->sel_y0, w->sel_x1, w->sel_y1);
        }

        glUniformMatrix4fv(w->GL_u_MVP_loc, 1, GL_FALSE, (const float *)w->GL_m_MVP_screen);

        /*glBindVertexArray(w->DBG_screen.VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, w->DBG_screen.EBO);
        draw_DBG(w->GL_u_color_loc, w->GL_u_flags_loc, w->GL_u_center_point_loc, &w->DBG_screen, wx0, wy0, wx1, wy1);*/

        gllc_W_cursor_draw(&w->cursor, w->GL_u_color_loc, w->cursor_x, w->cursor_y, w->width, w->height);

        gllc_WN_swap_buffers(w->native);
}

static void on_paint(struct gllc_WN *wn, void *USER_1)
{
        draw((struct gllc_window *)USER_1);
}

static void on_size(struct gllc_WN *wn, int width, int height, void *USER_1)
{
        gllc_WN_make_context_current(wn);

        glViewport(0, 0, width, height);

        ((struct gllc_window *)USER_1)->width = width;
        ((struct gllc_window *)USER_1)->height = height;

        update_viewport((struct gllc_window *)USER_1);
}

struct gllc_block *gllc_window_get_block(struct gllc_window *window)
{
        return window->block;
}

struct gllc_window *gllc_window_create(void *parent)
{
        fps_init();

        struct gllc_window *w = malloc(sizeof(struct gllc_window));
        if (!w)
        {
                goto _error;
        }

        memset(w, 0, sizeof(struct gllc_window));

        w->__obj.prop_def = g_props_def;

        w->native = gllc_WN_create(parent);

        if (!w->native)
        {
                goto _error;
        }

        gllc_WN_on_paint(w->native, on_paint, w);
        gllc_WN_on_size(w->native, on_size, w);
        gllc_WN_on_mouse_move(w->native, on_mouse_move, w);
        gllc_WN_on_mouse_scroll(w->native, on_mouse_scroll, w);
        gllc_WN_on_mouse_click(w->native, on_mouse_click, w);

        gllc_WN_make_context_current(w->native);

        gllc_W_grid_init(&w->grid);
        w->grid.color[0] = 0.5f;
        w->grid.color[1] = 0.5f;
        w->grid.color[2] = 0.5f;
        w->grid.color[3] = 0.3f;
        w->grid_used = 1;

        if (!gladLoadGL())
        {
                goto _error;
        }

        printf("Renderer: %s.\n", glGetString(GL_RENDERER));
        printf("OpenGL version supported %s.\n", glGetString(GL_VERSION));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        w->GL_program = load_GL_program();

        if (!w->GL_program)
        {
                goto _error;
        }

        glm_mat4_identity(w->GL_m_model);
        glm_mat4_identity(w->GL_m_view);
        glm_mat4_identity(w->GL_m_proj);
        glm_mat4_identity(w->GL_m_proj_screen);

        w->clear_color[0] = 0.0f;
        w->clear_color[1] = 0.0f;
        w->clear_color[2] = 0.0f;
        w->clear_color[3] = 1.0f;

        int width, height;
        gllc_WN_get_size(w->native, &width, &height);
        w->width = width;
        w->height = height;
        w->scale_factor = 1.0f;

        printf("Window size: %d, %d.\n", width, height);

        update_viewport(w);

        load_GL_uniform_loc(w);

        gllc_DBG_init(&w->DBG);
        gllc_DBG_init(&w->DBG_interactive);
        gllc_DBG_init(&w->DBG_screen);

        gllc_WN_make_context_current(0);

        return w;
_error:
        if (w)
        {
                if (w->native)
                {
                        gllc_WN_destroy(w->native);
                }
                free(w);
        }
        return 0;
}

void gllc_window_redraw(struct gllc_window *window)
{
        gllc_WN_dirty(window->native);
}

void gllc_window_set_clear_color(struct gllc_window *window, int r, int g, int b)
{
        gllc_WN_make_context_current(window->native);

        glClearColor((GLfloat)r / 255, (GLfloat)g / 255, (GLfloat)b / 255, 1.0f);

        window->clear_color[0] = (GLfloat)r / 255;
        window->clear_color[1] = (GLfloat)g / 255;
        window->clear_color[2] = (GLfloat)b / 255;
        window->clear_color[3] = 1.0f;

        struct gllc_W_grid_config conf = {
            .clear_color = window->clear_color};

        gllc_W_grid_configure(&window->grid, &conf);
}

void gllc_window_set_size(struct gllc_window *window, int x, int y, int width, int height)
{
        gllc_WN_set_size(window->native, x, y, width, height);

        gllc_WN_dirty(window->native);
}

void gllc_window_set_block(struct gllc_window *window,
                           struct gllc_block *block)
{
        window->block = block;
}
void gllc_window_destroy(struct gllc_window *window)
{
        gllc_WN_make_context_current(window->native);
        gllc_DBG_destroy(&window->DBG);
        gllc_DBG_destroy(&window->DBG_interactive);
        gllc_DBG_destroy(&window->DBG_screen);
        gllc_W_grid_cleanup(&window->grid);
        gllc_WN_destroy(window->native);

        free(window);
}

void gllc_window_enable_grid(struct gllc_window *window, int enable)
{
        window->grid_used = enable;
}

void gllc_window_grid_configure(struct gllc_window *window, double gap_x, double gap_y, double offset_x, double offset_y, float *color)
{
        window->grid.offset_x = offset_x;
        window->grid.offset_y = offset_y;
        window->grid.gap_x = gap_x;
        window->grid.gap_y = gap_y;

        memcpy(window->grid.color, color, sizeof(float) * 4);
}

void gllc_window_wnd_to_drw(struct gllc_window *w, double x, double y, double *xd, double *yd)
{
        *xd = (x - ((double)w->width / 2)) * w->scale_factor - w->dx;
        *yd = (y - ((double)w->height / 2)) * w->scale_factor - w->dy;
}

void gllc_window_zoom_bb(struct gllc_window *window)
{
        if (!window->block)
        {
                return;
        }

        double x_min, y_min, x_max, y_max;

        if (window->block->DBD.DE_count == 0)
        {
                return;
        }

        struct gllc_DE *DE = window->block->DBD.DE_head;

        x_min = DE->BBox_x0;
        y_min = DE->BBox_y0;
        x_max = DE->BBox_x1;
        y_max = DE->BBox_y1;
        DE = DE->next;

        while (DE)
        {
                if (DE->BBox_x0 < x_min)
                        x_min = DE->BBox_x0;
                if (DE->BBox_y0 < y_min)
                        y_min = DE->BBox_y0;
                if (DE->BBox_x1 > x_max)
                        x_max = DE->BBox_x1;
                if (DE->BBox_y1 > y_max)
                        y_max = DE->BBox_y1;

                DE = DE->next;
        }

        if (fabs(x_max - x_min) > fabs(y_max - y_min))
        {
                window->scale_factor = fabs(x_max - x_min) / (double)window->width;
        }
        else
        {
                window->scale_factor = fabs(y_max - y_min) / (double)window->height;
        }

        window->dx = -(x_min + ((x_max - x_min) / 2.0f));
        window->dy = -(y_min + ((y_max - y_min) / 2.0f));

        update_viewport(window);

        gllc_WN_dirty(window->native);
}