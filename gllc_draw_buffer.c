#include "gllc_draw_buffer.h"
#include "glad.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void push_DE(struct gllc_DBD *DBD, struct gllc_DE *DE)
{
        assert(DBD);
        assert(DE);

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

struct gllc_DE *gllc_DE_create(struct gllc_DBD *DBD, GLenum GL_type)
{
        struct gllc_DE *DE = malloc(sizeof(struct gllc_DE));
        if (DE)
        {
                memset(DE, 0, sizeof(struct gllc_DE));

                DE->GL_type = GL_type;
                DE->DBD = DBD;

                push_DE(DBD, DE);
        }
        return DE;
}

void gllc_DE_skip(struct gllc_DE *DE, int skip)
{
        DE->skip = skip;
        DE->DBD->modified = 1;
}

int gllc_DE_update(struct gllc_DE *DE, struct gllc_DE_config *DE_config)
{
        assert(DE);
        assert(DE_config);
        assert(DE->DBD);
        GLfloat *V_new = NULL;
        GLuint *I_new = NULL;

        if (DE_config->V)
        {
                size_t size = sizeof(GLfloat) * 2 * DE_config->V_count;
                V_new = malloc(size);
                if (!V_new)
                        return 0;
                memcpy(V_new, DE_config->V, size);
        }

        if (DE_config->I)
        {
                size_t size = sizeof(GLuint) * DE_config->I_count;
                I_new = malloc(size);
                if (!I_new)
                {
                        free(V_new);
                        return 0;
                }
                memcpy(I_new, DE_config->I, size);
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
                free(DE->V_cache);
                DE->V_cache = V_new;
                DE->V_cache_count = DE_config->V_count;

                if (DE->V_cache_count > 0)
                {
                        GLuint i;
                        GLfloat x, y;
                        DE->BBox_x0 = DE->V_cache[0];
                        DE->BBox_y0 = DE->V_cache[1];
                        DE->BBox_x1 = DE->V_cache[0];
                        DE->BBox_y1 = DE->V_cache[1];

                        for (i = 1; i < DE->V_cache_count; i++)
                        {
                                x = DE->V_cache[i * 2];
                                y = DE->V_cache[i * 2 + 1];
                                if (x < DE->BBox_x0)
                                        DE->BBox_x0 = x;
                                if (y < DE->BBox_y0)
                                        DE->BBox_y0 = y;
                                if (x > DE->BBox_x1)
                                        DE->BBox_x1 = x;
                                if (y > DE->BBox_y1)
                                        DE->BBox_y1 = y;
                        }
                }
        }

        if (I_new)
        {
                free(DE->I_cache);
                DE->I_cache = I_new;
                DE->I_cache_count = DE_config->I_count;
        }

        if (DE->DBD)
                DE->DBD->modified = 1;

        return 1;
}

void gllc_DBD_init(struct gllc_DBD *DBD)
{
        memset(DBD, 0, sizeof(struct gllc_DBD));
}

void gllc_DBG_init(struct gllc_DBG *DBG)
{
        memset(DBG, 0, sizeof(*DBG));
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
        free(DBG->V);
        free(DBG->I);
        memset(DBG, 0, sizeof(struct gllc_DBG));
}

void gllc_DBG_build(struct gllc_DBG *DBG, struct gllc_DBD *DBD)
{
        assert(DBG);
        assert(DBD);

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
                        VBO_size += sizeof(GLfloat) * 2 * DE->V_cache_count;
                        EBO_size += sizeof(GLuint) * DE->I_cache_count;
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

        if (DBG->V_size < VBO_size)
        {
                void *new_V = malloc(VBO_size);
                if (!new_V)
                        return;

                free(DBG->V);

                DBG->V = new_V;
                DBG->V_size = VBO_size;
        }

        if (DBG->I_size < EBO_size)
        {
                void *new_I = malloc(EBO_size);
                if (!new_I)
                        return;

                free(DBG->I);

                DBG->I = new_I;
                DBG->I_size = EBO_size;
        }

        glBindBuffer(GL_ARRAY_BUFFER, DBG->VBO);
        glBufferData(GL_ARRAY_BUFFER, VBO_size, NULL, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DBG->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, EBO_size, NULL, GL_STATIC_DRAW);

        GLuint VBO_offset = 0;
        GLuint EBO_offset = 0;
        int V_nth = 0, I_nth = 0;
        DE = DBD->DE_head;

        int i = 0, j;
        while (DE)
        {
                if (DE->skip)
                {
                        DE = DE->next;
                        continue;
                }

                memcpy(DBG->V + VBO_offset, DE->V_cache, sizeof(GLfloat) * 2 * DE->V_cache_count);

                for (j = 0; j < DE->I_cache_count; j++)
                {
                        DE->I_cache[j] += V_nth;
                }

                memcpy(DBG->I + EBO_offset, DE->I_cache, sizeof(GLuint) * DE->I_cache_count);

                for (j = 0; j < DE->I_cache_count; j++)
                {
                        DE->I_cache[j] -= V_nth;
                }

                DBG->DE[i].GL_type = DE->GL_type;
                DBG->DE[i].offset = I_nth;
                DBG->DE[i].size = DE->I_cache_count;
                DBG->DE[i].color[0] = DE->color[0];
                DBG->DE[i].color[1] = DE->color[1];
                DBG->DE[i].color[2] = DE->color[2];
                DBG->DE[i].color[3] = DE->color[3];
                DBG->DE[i].atlas_index = 0;
                DBG->DE[i].tex_u0 = 0;
                DBG->DE[i].tex_u1 = 0;
                DBG->DE[i].tex_v0 = 0;
                DBG->DE[i].tex_v1 = 0;
                DBG->DE[i].BBox_x0 = DE->BBox_x0;
                DBG->DE[i].BBox_y0 = DE->BBox_y0;
                DBG->DE[i].BBox_x1 = DE->BBox_x1;
                DBG->DE[i].BBox_y1 = DE->BBox_y1;
                VBO_offset += sizeof(GLfloat) * 2 * DE->V_cache_count;
                EBO_offset += sizeof(GLuint) * DE->I_cache_count;
                V_nth += DE->V_cache_count;
                I_nth += DE->I_cache_count;
                i++;
                DE = DE->next;
        }

        glBufferSubData(GL_ARRAY_BUFFER, 0, VBO_size, DBG->V);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, EBO_size, DBG->I);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        DBG->DE_size = DE_size;

        DBD->modified = 0;
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
        if (DE->V_cache)
                free(DE->V_cache);
        if (DE->I_cache)
                free(DE->I_cache);

        remove_DE(DE->DBD, DE);

        free(DE);
}