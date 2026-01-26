#ifndef gllc_draw_buffer_h
#define gllc_draw_buffer_h

#include "glad.h"

struct gllc_DE_config
{
        const int *skip;
        const GLfloat *v;
        const GLuint *i;
        const GLfloat *color;
        GLuint v_count;
        GLuint i_count;
};

struct gllc_DBD;

struct gllc_DE
{
        struct gllc_DBD *DBD;
        int skip;
        GLfloat *v_cache;
        GLuint *i_cache;
        GLuint v_cache_count;
        GLuint i_cache_count;
        GLfloat color[4];
        struct gllc_DE *next;
        struct gllc_DE *prev;
};

struct gllc_DBD
{
        int modified;
        GLenum GL_type;
        struct gllc_DE *DE_head;
        struct gllc_DE *DE_tail;
        size_t DE_count;
};

struct gllc_DBG_DE
{
        GLuint offset;
        GLuint size;
        GLfloat color[4];
        GLfloat tex_u0;
        GLfloat tex_v0;
        GLfloat tex_u1;
        GLfloat tex_v1;
        GLuint atlas_index;
};

struct gllc_DBG
{
        GLenum GL_type;
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
        GLuint VBO_size;
        GLuint EBO_size;
        struct gllc_DBG_DE *DE;
        size_t DE_cap;
        size_t DE_size;
};

struct gllc_DBD_batch
{
        struct gllc_DBD GL_lines;
        struct gllc_DBD GL_line_strip;
        struct gllc_DBD GL_line_loop;
        struct gllc_DBD GL_triangles;
        struct gllc_DBD GL_triangle_strip;
        struct gllc_DBD GL_triangle_fan;
        struct gllc_DBD GL_points;
};

struct gllc_DBG_batch
{
        struct gllc_DBG GL_lines;
        struct gllc_DBG GL_line_strip;
        struct gllc_DBG GL_line_loop;
        struct gllc_DBG GL_triangles;
        struct gllc_DBG GL_triangle_strip;
        struct gllc_DBG GL_triangle_fan;
        struct gllc_DBG GL_points;
};

struct gllc_DE *gllc_DE_create(struct gllc_DBD *DBD);

int gllc_DE_update(struct gllc_DE *DE, struct gllc_DE_config *DE_config);

void gllc_DBD_batch_init(struct gllc_DBD_batch *DBD_batch);

void gllc_DBG_init(struct gllc_DBG *DBG, GLenum GL_type);

void gllc_DBG_batch_init(struct gllc_DBG_batch *DBG_batch);

void gllc_DBG_destroy(struct gllc_DBG *DBG);

void gllc_DBG_batch_destroy(struct gllc_DBG_batch *DBG_batch);

void gllc_DBD_destroy(struct gllc_DBD *DBD);

void gllc_DBD_batch_destroy(struct gllc_DBD_batch *DBD_batch);

void gllc_DBG_build(struct gllc_DBG *DBG, struct gllc_DBD *DBD);

void gllc_DBG_batch_build(struct gllc_DBG_batch *DBG_batch, struct gllc_DBD_batch *DBD_batch);

void gllc_DE_destroy(struct gllc_DE *DE);

void gllc_DBD_batch_modified(struct gllc_DBD_batch *DBD_batch);

#endif