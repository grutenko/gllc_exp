#ifndef gllc_draw_buffer_h
#define gllc_draw_buffer_h

#include "glad.h"

struct gllc_DE_config
{
        const GLfloat *color;
        const GLfloat *V;
        const GLuint *I;
        GLuint V_count;
        GLuint I_count;
};

struct gllc_DBD;

struct gllc_DE
{
        struct gllc_DBD *DBD;
        GLenum GL_type;
        int layer;
        int skip;
        GLfloat *V_cache;
        GLuint *I_cache;
        GLuint V_cache_count;
        GLuint I_cache_count;
        GLfloat color[4];
        struct gllc_DE *next;
        struct gllc_DE *prev;
};

struct gllc_DBD
{
        int modified;
        struct gllc_DE *DE_head;
        struct gllc_DE *DE_tail;
        size_t DE_count;
};

struct gllc_DBG_DE
{
        GLenum GL_type;
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
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
        GLuint VBO_size;
        GLuint EBO_size;
        struct gllc_DBG_DE *DE;
        size_t DE_cap;
        size_t DE_size;
        void *V;
        void *I;
        size_t V_size;
        size_t I_size;
};

struct gllc_DE *gllc_DE_create(struct gllc_DBD *DBD, GLenum GL_type);

void gllc_DBD_init(struct gllc_DBD *DBD);

int gllc_DE_update(struct gllc_DE *DE, struct gllc_DE_config *DE_config);

void gllc_DE_skip(struct gllc_DE *DE, int skip);

void gllc_DBG_init(struct gllc_DBG *DBG);

void gllc_DBG_destroy(struct gllc_DBG *DBG);

void gllc_DBD_destroy(struct gllc_DBD *DBD);

void gllc_DBG_build(struct gllc_DBG *DBG, struct gllc_DBD *DBD);

void gllc_DE_destroy(struct gllc_DE *DE);

#endif