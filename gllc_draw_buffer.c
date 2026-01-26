#include "gllc_draw_buffer.h"
#include "glad.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void push_DE(struct gllc_DBD *DBD, struct gllc_DE *DE)
{
        assert(DBD);
        assert(DE);
        assert(!DE->DBD);
        if (!DBD->DE_head)
        {
                DBD->DE_head = DE;
        }
        else
        {
                DE->prev = DBD->DE_tail;
                DBD->DE_tail->next = DE;
        }
        DBD->DE_tail = DE;
        DBD->DE_count++;
}

struct gllc_DE *gllc_DE_create(struct gllc_DBD *DBD)
{
        struct gllc_DE *DE = malloc(sizeof(struct gllc_DE));
        if (DE)
        {
                DE->color[0] = 0.0f;
                DE->color[1] = 0.0f;
                DE->color[2] = 0.0f;
                DE->color[3] = 0.0f;
                DE->i_cache = 0ULL;
                DE->i_cache_count = 0ULL;
                DE->v_cache = 0ULL;
                DE->v_cache_count = 0ULL;
                DE->next = 0ULL;
                DE->prev = 0ULL;
                push_DE(DBD, DE);
                DE->DBD = DBD;
        }
        return DE;
}

int gllc_DE_update(struct gllc_DE *DE, struct gllc_DE_config *DE_config)
{
        assert(DE);
        assert(DE_config);
        assert(DE->DBD);
        GLfloat *V_new = NULL;
        GLuint *I_new = NULL;

        if (DE_config->skip)
        {
                DE->skip = *DE_config->skip;
        }

        if (DE_config->v)
        {
                size_t size = sizeof(GLfloat) * 2 * DE_config->v_count;
                V_new = malloc(size);
                if (!V_new)
                        return 0;
                memcpy(V_new, DE_config->v, size);
        }

        if (DE_config->i)
        {
                size_t size = sizeof(GLuint) * DE_config->i_count;
                I_new = malloc(size);
                if (!I_new)
                {
                        free(V_new);
                        return 0;
                }
                memcpy(I_new, DE_config->i, size);
        }

        if (DE_config->color)
        {
                DE->color[0] = DE_config->color[0];
                DE->color[1] = DE_config->color[1];
                DE->color[2] = DE_config->color[2];
                DE->color[3] = DE_config->color[3];
        }

        if (V_new)
        {
                free(DE->v_cache);
                DE->v_cache = V_new;
                DE->v_cache_count = DE_config->v_count;
        }

        if (I_new)
        {
                free(DE->i_cache);
                DE->i_cache = I_new;
                DE->i_cache_count = DE_config->i_count;
        }

        if (DE->DBD)
                DE->DBD->modified = 1;

        return 1;
}

void gllc_DBD_init(struct gllc_DBD *DBD, int GL_type)
{
        memset(DBD, 0, sizeof(struct gllc_DBD));
        DBD->GL_type = GL_type;
}

void gllc_DBD_batch_init(struct gllc_DBD_batch *DBD_batch)
{
        gllc_DBD_init(&DBD_batch->GL_lines, GL_LINES);
        gllc_DBD_init(&DBD_batch->GL_line_strip, GL_LINE_STRIP);
        gllc_DBD_init(&DBD_batch->GL_line_loop, GL_LINE_LOOP);
        gllc_DBD_init(&DBD_batch->GL_triangles, GL_TRIANGLES);
        gllc_DBD_init(&DBD_batch->GL_triangle_strip, GL_TRIANGLE_STRIP);
        gllc_DBD_init(&DBD_batch->GL_triangle_fan, GL_TRIANGLE_FAN);
        gllc_DBD_init(&DBD_batch->GL_points, GL_POINTS);
}

void gllc_DBG_init(struct gllc_DBG *DBG, GLenum GL_type)
{
        memset(DBG, 0, sizeof(*DBG));
        DBG->GL_type = GL_type;
        glGenVertexArrays(1, &DBG->VAO);
        glBindVertexArray(DBG->VAO);

        glGenBuffers(1, &DBG->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, DBG->VBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void *)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &DBG->EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DBG->EBO);

        DBG->VBO_size = 0;
        DBG->EBO_size = 0;

        glBindVertexArray(0);
}

void gllc_DBG_batch_init(struct gllc_DBG_batch *DBG_batch)
{
        gllc_DBG_init(&DBG_batch->GL_lines, GL_LINES);
        gllc_DBG_init(&DBG_batch->GL_line_strip, GL_LINE_STRIP);
        gllc_DBG_init(&DBG_batch->GL_line_loop, GL_LINE_LOOP);
        gllc_DBG_init(&DBG_batch->GL_triangles, GL_TRIANGLES);
        gllc_DBG_init(&DBG_batch->GL_triangle_strip, GL_TRIANGLE_STRIP);
        gllc_DBG_init(&DBG_batch->GL_triangle_fan, GL_TRIANGLE_FAN);
        gllc_DBG_init(&DBG_batch->GL_points, GL_POINTS);
}

void gllc_DBD_destroy(struct gllc_DBD *DBD)
{
        struct gllc_DE *DE = DBD->DE_head;
        while (DE)
        {
                struct gllc_DE *DE_next = DE->next;
                gllc_DE_destroy(DE);
                DE = DE_next;
        }
        DBD->DE_head = NULL;
        DBD->DE_tail = NULL;
        DBD->DE_count = 0;
        DBD->modified = 1;
}

void gllc_DBD_batch_destroy(struct gllc_DBD_batch *DBD_batch)
{
        gllc_DBD_destroy(&DBD_batch->GL_line_loop);
        gllc_DBD_destroy(&DBD_batch->GL_line_strip);
        gllc_DBD_destroy(&DBD_batch->GL_lines);
        gllc_DBD_destroy(&DBD_batch->GL_triangle_strip);
        gllc_DBD_destroy(&DBD_batch->GL_triangles);
        gllc_DBD_destroy(&DBD_batch->GL_triangle_fan);
        gllc_DBD_destroy(&DBD_batch->GL_points);
}

void gllc_DBG_destroy(struct gllc_DBG *DBG)
{
        if (DBG->EBO)
        {
                glDeleteBuffers(1, &DBG->EBO);
        }
        if (DBG->VBO)
        {
                glDeleteBuffers(1, &DBG->VBO);
        }
        if (DBG->VAO)
        {
                glDeleteVertexArrays(1, &DBG->VAO);
        }
        free(DBG->DE);
        memset(DBG, 0, sizeof(struct gllc_DBG));
}

void gllc_DBG_batch_destroy(struct gllc_DBG_batch *DBG_batch)
{
        gllc_DBG_destroy(&DBG_batch->GL_lines);
        gllc_DBG_destroy(&DBG_batch->GL_line_strip);
        gllc_DBG_destroy(&DBG_batch->GL_line_loop);
        gllc_DBG_destroy(&DBG_batch->GL_triangles);
        gllc_DBG_destroy(&DBG_batch->GL_triangle_strip);
        gllc_DBG_destroy(&DBG_batch->GL_triangle_fan);
        gllc_DBG_destroy(&DBG_batch->GL_points);
}

void gllc_DBG_build(struct gllc_DBG *DBG, struct gllc_DBD *DBD)
{
        assert(DBG);
        assert(DBD);
        assert(DBG->GL_type == DBD->GL_type);

        if (!DBD->modified)
                return;

        GLuint VBO_size = 0;
        GLuint EBO_size = 0;
        size_t DE_size = 0;
        struct gllc_DE *DE = DBD->DE_head;
        while (DE)
        {
                if (!DE->skip)
                {
                        VBO_size += sizeof(GLfloat) * 2 * DE->v_cache_count;
                        EBO_size += sizeof(GLuint) * DE->i_cache_count;
                        DE_size++;
                }
                DE = DE->next;
        }

        if (DBG->DE_cap <= DE_size)
        {
                struct gllc_DBG_DE *DE_new = realloc(DBG->DE, sizeof(struct gllc_DBG_DE) * DE_size);
                if (!DE_new)
                        return;
                DBG->DE = DE_new;
                DBG->DE_cap = DE_size;
        }

        glBindBuffer(GL_ARRAY_BUFFER, DBG->VBO);
        glBufferData(GL_ARRAY_BUFFER, VBO_size, NULL, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DBG->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, EBO_size, NULL, GL_STATIC_DRAW);

        GLuint VBO_offset = 0;
        GLuint EBO_offset = 0;
        DE = DBD->DE_head;

        int i = 0;
        while (DE)
        {
                if (!DE->skip)
                {
                        glBufferSubData(GL_ARRAY_BUFFER, VBO_offset, sizeof(GLfloat) * 2 * DE->v_cache_count, DE->v_cache);

                        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, EBO_offset, sizeof(GLuint) * DE->i_cache_count, DE->i_cache);
                        
                        DBG->DE[i].offset = EBO_offset;
                        DBG->DE[i].size = DE->i_cache_count;
                        DBG->DE[i].color[0] = DE->color[0];
                        DBG->DE[i].color[1] = DE->color[1];
                        DBG->DE[i].color[2] = DE->color[2];
                        DBG->DE[i].color[3] = DE->color[3];
                        DBG->DE[i].atlas_index = 0;
                        DBG->DE[i].tex_u0 = 0;
                        DBG->DE[i].tex_u1 = 0;
                        DBG->DE[i].tex_v0 = 0;
                        DBG->DE[i].tex_v1 = 0;
                        VBO_offset += sizeof(GLfloat) * 2 * DE->v_cache_count;
                        EBO_offset += sizeof(GLuint) * DE->i_cache_count;
                        i++;
                }
                DE = DE->next;
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        DBG->DE_size = DE_size;

        DBD->modified = 0;
}

void gllc_DBG_batch_build(struct gllc_DBG_batch *DBG_batch, struct gllc_DBD_batch *DBD_batch)
{
        gllc_DBG_build(&DBG_batch->GL_line_loop, &DBD_batch->GL_line_loop);
        gllc_DBG_build(&DBG_batch->GL_line_strip, &DBD_batch->GL_line_strip);
        gllc_DBG_build(&DBG_batch->GL_lines, &DBD_batch->GL_lines);
        gllc_DBG_build(&DBG_batch->GL_triangles, &DBD_batch->GL_triangles);
        gllc_DBG_build(&DBG_batch->GL_triangle_strip, &DBD_batch->GL_triangle_strip);
        gllc_DBG_build(&DBG_batch->GL_triangle_fan, &DBD_batch->GL_triangle_fan);
        gllc_DBG_build(&DBG_batch->GL_points, &DBD_batch->GL_points);
}

static void remove_DE(struct gllc_DBD *DBD, struct gllc_DE *DE)
{
        assert(DBD);
        assert(DE);
        assert(DE->DBD == DBD);

        if (DE->prev)
                DE->prev->next = DE->next;
        if (DE->next)
                DE->next->prev = DE->prev;
        if (!DE->prev)
                DBD->DE_head = DE->next;
        if (!DE->next)
                DBD->DE_tail = DE->prev;

        DBD->DE_count--;

        DE->next = NULL;
        DE->prev = NULL;
        DE->DBD = NULL;
}

void gllc_DE_destroy(struct gllc_DE *DE)
{
        if (!DE)
                return;
        if (DE->v_cache)
                free(DE->v_cache);
        if (DE->i_cache)
                free(DE->i_cache);
        remove_DE(DE->DBD, DE);
}

void gllc_DBD_batch_modified(struct gllc_DBD_batch *DBD_batch)
{
        DBD_batch->GL_lines.modified = 1;
        DBD_batch->GL_line_strip.modified = 1;
        DBD_batch->GL_line_loop.modified = 1;
        DBD_batch->GL_triangles.modified = 1;
        DBD_batch->GL_triangle_strip.modified = 1;
        DBD_batch->GL_triangle_fan.modified = 1;
        DBD_batch->GL_points.modified = 1;
}