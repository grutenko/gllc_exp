#include "gllc_window.h"
#include "glad.h"
#include "gllc_block.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"
#include "gllc_window_grid.h"
#include "gllc_window_native.h"

#include "gllc_fragment_shader.h"
#include "gllc_vertex_shader.h"

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

static void render_camera(struct gllc_window *w)
{
        float half_w = (float)w->width / 2.0f * w->scale_factor;
        float half_h = (float)w->height / 2.0f * w->scale_factor;

        glm_ortho(-half_w, half_w, half_h, -half_h, -1000.0f, 1000.0f, w->GL_m_proj);

        glm_ortho(0.0f, (float)w->width, (float)w->height, 0.0f, -1.0f, 1.0f, w->GL_m_proj_screen);

        glm_mat4_identity(w->GL_m_view);
        vec4 t = {w->dx, w->dy, 0.0f, 1.0f};
        glm_translate(w->GL_m_view, t);

        glm_mat4_mul(w->GL_m_proj, w->GL_m_view, w->GL_m_MVP);
        glm_mat4_mul(w->GL_m_MVP, w->GL_m_model, w->GL_m_MVP);

        glm_mat4_copy(w->GL_m_proj_screen, w->GL_m_MVP_screen);
}

static void load_GL_uniform_loc(struct gllc_window *w)
{
        w->GL_u_MVP_loc = glGetUniformLocation(w->GL_program, "uMVP");
        if (w->GL_u_MVP_loc == -1)
        {
                fprintf(stderr, "Uniform uMVP not found!\n");
        }
        GLint uColor_loc = glGetUniformLocation(w->GL_program, "uColor");
        if (uColor_loc == -1)
        {
                fprintf(stderr, "Uniform uColor not found!\n");
        }
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
}

static void on_mouse_move(struct gllc_WN *wn, int x, int y, void *USER_1)
{
        if (g_drag.dragging)
        {
                int dx = x - g_drag.last_x;
                int dy = y - g_drag.last_y;

                struct gllc_window *w = (struct gllc_window *)USER_1;
                w->dx += dx * w->scale_factor;
                w->dy += dy * w->scale_factor;

                g_drag.last_x = x;
                g_drag.last_y = y;

                render_camera(w);

                gllc_WN_dirty(wn);
        }
}

static void on_mouse_scroll(struct gllc_WN *wn, int dx, int dy, void *USER_1)
{
        struct gllc_window *w = (struct gllc_window *)USER_1;

        if (dy > 2.0f)
                dy = 2.0f;
        if (dy < -10.0f)
                dy = -2.0f;

        double old_scale = w->scale_factor;
        w->scale_factor *= 1.0f + ((double)dy * 0.1);

        int mouse_x, mouse_y;
        gllc_WN_get_cursor(wn, &mouse_x, &mouse_y);

        w->dx += (mouse_x - (int)(w->width / 2)) * (w->scale_factor - old_scale);
        w->dy += (mouse_y - (int)(w->height / 2)) * (w->scale_factor - old_scale);

        render_camera((struct gllc_window *)USER_1);

        gllc_WN_dirty(wn);
}

static size_t draw_DBG(GLuint color_loc, struct gllc_DBG *DBG)
{
        int i;
        for (i = 0; i < DBG->DE_size; i++)
        {
                glUniform4f(color_loc,
                            DBG->DE[i].color[0],
                            DBG->DE[i].color[1],
                            DBG->DE[i].color[2],
                            DBG->DE[i].color[3]);
                glDrawElements(DBG->GL_type,
                               DBG->DE[i].size,
                               GL_UNSIGNED_INT,
                               (void *)(sizeof(GLuint) * DBG->DE[i].offset));
        }
        return DBG->DE_size;
}

static void draw(struct gllc_window *w)
{
        if (w->block)
        {
                gllc_DBG_batch_build(&w->DBG_batch, &w->block->DBD_batch);
        }

        int i;
        mat4 m_identity;
        glm_mat4_identity(m_identity);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(w->GL_program);
        glUniformMatrix4fv(w->GL_u_MVP_loc, 1, GL_FALSE, (const float *)w->GL_m_MVP);

        if (w->grid_used)
        {
                double x0 = -(int)(w->width / 2) * w->scale_factor - w->dx;
                double y0 = -(int)(w->height / 2) * w->scale_factor - w->dy;
                double x1 = (int)(w->width / 2) * w->scale_factor - w->dx;
                double y1 = (int)(w->height / 2) * w->scale_factor - w->dy;
                gllc_W_grid_draw(&w->grid, w->GL_u_color_loc, x0, y0, x1, y1, w->scale_factor);
        }

        size_t draw_calls = 0;

        for (i = 0; i < GLLC_DRAW_BUFFERS; i++)
        {
                if (w->DBG_order[i]->DE_size == 0)
                {
                        continue;
                }

                glBindVertexArray(w->DBG_order[i]->VAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, w->DBG_order[i]->EBO);

                draw_calls += draw_DBG(w->GL_u_color_loc, w->DBG_order[i]);
        }

        glUniformMatrix4fv(w->GL_u_MVP_loc, 1, GL_FALSE, (const float *)w->GL_m_MVP_screen);

        for (i = 0; i < 7; i++)
        {
                if (w->DBG_order_screen[i]->DE_size == 0)
                {
                        continue;
                }

                glBindVertexArray(w->DBG_order_screen[i]->VAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, w->DBG_order[i]->EBO);

                draw_calls += draw_DBG(w->GL_u_color_loc, w->DBG_order_screen[i]);
        }

        gllc_WN_swap_buffers(w->native);
}

static void on_paint(struct gllc_WN *wn, void *USER_1)
{
        draw((struct gllc_window *)USER_1);
}

static void on_size(struct gllc_WN *wn, int width, int height, void *USER_1)
{
        ((struct gllc_window *)USER_1)->width = width;
        ((struct gllc_window *)USER_1)->height = height;

        render_camera((struct gllc_window *)USER_1);
}

static void set_DBG_order(struct gllc_window *w)
{
        w->DBG_order[0] = &w->DBG_batch.GL_triangles;
        w->DBG_order[1] = &w->DBG_batch.GL_triangle_strip;
        w->DBG_order[2] = &w->DBG_batch.GL_triangle_fan;
        w->DBG_order[3] = &w->DBG_batch.GL_lines;
        w->DBG_order[4] = &w->DBG_batch.GL_line_strip;
        w->DBG_order[5] = &w->DBG_batch.GL_line_loop;
        w->DBG_order[6] = &w->DBG_batch.GL_points;
        w->DBG_order[7] = &w->DBG_batch_interactive.GL_triangles;
        w->DBG_order[8] = &w->DBG_batch_interactive.GL_triangle_strip;
        w->DBG_order[9] = &w->DBG_batch_interactive.GL_triangle_fan;
        w->DBG_order[10] = &w->DBG_batch_interactive.GL_lines;
        w->DBG_order[11] = &w->DBG_batch_interactive.GL_line_strip;
        w->DBG_order[12] = &w->DBG_batch_interactive.GL_line_loop;
        w->DBG_order[13] = &w->DBG_batch_interactive.GL_points;
        w->DBG_order_screen[0] = &w->DBG_batch_screen.GL_triangles;
        w->DBG_order_screen[1] = &w->DBG_batch_screen.GL_triangle_strip;
        w->DBG_order_screen[2] = &w->DBG_batch_screen.GL_triangle_fan;
        w->DBG_order_screen[3] = &w->DBG_batch_screen.GL_lines;
        w->DBG_order_screen[4] = &w->DBG_batch_screen.GL_line_strip;
        w->DBG_order_screen[5] = &w->DBG_batch_screen.GL_line_loop;
        w->DBG_order_screen[6] = &w->DBG_batch_screen.GL_points;
}

struct gllc_block *gllc_window_get_block(struct gllc_window *window)
{
        return window->block;
}

struct gllc_window *gllc_window_create(void *parent)
{
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
        w->grid.color[0] = 0.9f;
        w->grid.color[1] = 0.9f;
        w->grid.color[2] = 0.9f;
        w->grid.color[3] = 1.0f;
        w->grid_used = 1;

        if (!gladLoadGL())
        {
                goto _error;
        }

        printf("Renderer: %s.\n", glGetString(GL_RENDERER));
        printf("OpenGL version supported %s.\n", glGetString(GL_VERSION));

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

        int width, height;
        gllc_WN_get_size(w->native, &width, &height);
        w->width = width;
        w->height = height;
        w->scale_factor = 1.0f;

        printf("Window size: %d, %d.\n", width, height);

        render_camera(w);

        load_GL_uniform_loc(w);

        gllc_DBG_batch_init(&w->DBG_batch);
        gllc_DBG_batch_init(&w->DBG_batch_interactive);
        gllc_DBG_batch_init(&w->DBG_batch_screen);

        set_DBG_order(w);

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
}

void gllc_window_set_size(struct gllc_window *window, int x, int y, int width, int height)
{
        gllc_WN_set_size(window->native, x, y, width, height);
}

void gllc_window_set_block(struct gllc_window *window,
                           struct gllc_block *block)
{
        window->block = block;
        if (window->block)
        {
                gllc_DBD_batch_modified(&window->block->DBD_batch);
        }
}
void gllc_window_destroy(struct gllc_window *window)
{
        gllc_WN_make_context_current(window->native);
        gllc_DBG_batch_destroy(&window->DBG_batch);
        gllc_DBG_batch_destroy(&window->DBG_batch_interactive);
        gllc_DBG_batch_destroy(&window->DBG_batch_screen);
        gllc_W_grid_cleanup(&window->grid);
        gllc_WN_destroy(window->native);

        free(window);
}