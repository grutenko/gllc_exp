#include "gllc_polyline.h"
#include "gllc_block.h"
#include "gllc_block_entity.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct gllc_prop_def g_internal_props_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_block_entity_prop_def,
                                                    g_internal_props_def,
                                                    0};

static void destruct(struct gllc_block_entity *ent)
{
        struct gllc_polyline *pline = (struct gllc_polyline *)ent;
        if (pline->DE_bound)
                gllc_DE_destroy(pline->DE_bound);

        free(pline->ver);
}

static GLfloat *G_vertices = NULL;
static GLuint *G_indices = NULL;
static size_t G_vcap = 0;
static size_t G_icap = 0;

static int reserve(void **b, size_t *cap, size_t new_size)
{
        if (*cap >= new_size)
        {
                return 1;
        }
        void *new_b = malloc(new_size);
        if (!new_b)
        {
                return 0;
        }
        free(*b);
        *cap = new_size;
        *b = new_b;

        return 1;
}

static int reserve_V(size_t size)
{
        return reserve((void **)&G_vertices, &G_vcap, size);
}

static int reserve_I(size_t size)
{
        return reserve((void **)&G_indices, &G_icap, size);
}

void gllc_polyline_build(const struct gllc_polyline_vertex *V, size_t V_count, struct gllc_DE *DE_bound, struct gllc_DE *DE_fill, float *color, float *fcolor, int closed, int filled)
{
        size_t V_size = sizeof(GLfloat) * 2 * V_count;
        size_t I_size = sizeof(GLuint) * V_count;

        if (!reserve_V(V_size) || !reserve_I(I_size))
                return;

        int i;
        for (i = 0; i < V_count; i++)
        {
                G_vertices[i * 2] = (GLfloat)V[i].x;
                G_vertices[i * 2 + 1] = (GLfloat)V[i].y;
                G_indices[i] = i;
        }

        struct gllc_DE_config DE_config = {
            .V = G_vertices,
            .I = G_indices,
            .V_count = V_count,
            .I_count = V_count,
            .color = color};

        gllc_DE_update(DE_bound, &DE_config);

        if (closed && filled)
        {
                // Triangulate and push DE triangles
        }
}

static void build(struct gllc_block_entity *ent, struct gllc_DBD *DBD)
{
        struct gllc_polyline *pline = (struct gllc_polyline *)ent;

        if (pline->closed && pline->filled)
        {
                pline->DE_fill = gllc_DE_create(DBD, GL_TRIANGLES);

                if (!pline->DE_fill)
                {
                        return;
                }
        }
        else
        {
                if (pline->DE_fill)
                {
                        gllc_DE_destroy(pline->DE_fill);
                }

                pline->DE_fill = NULL;
        }

        if (!pline->DE_bound)
        {
                if (pline->closed)
                {
                        pline->DE_bound = gllc_DE_create(DBD, GL_LINE_LOOP);

                        if (!pline->DE_bound)
                        {
                                return;
                        }
                }
                else
                {
                        pline->DE_bound = gllc_DE_create(DBD, GL_LINE_STRIP);

                        if (!pline->DE_bound)
                        {
                                return;
                        }
                }
        }

        if (pline->closed)
        {
                pline->DE_bound->GL_type = GL_LINE_LOOP;
        }
        else
        {
                pline->DE_bound->GL_type = GL_LINE_STRIP;
        }

        int color = gllc_block_entity_color(ent);
        int fcolor = gllc_block_entity_fcolor(ent);

        GLfloat color_[] = {
            (GLfloat)((color >> 16) & 0xff) / 255,
            (GLfloat)((color >> 8) & 0xff) / 255,
            (GLfloat)(color & 0xff) / 255,
            1.0f};

        GLfloat fcolor_[] = {
            (GLfloat)((fcolor >> 16) & 0xff) / 255,
            (GLfloat)((fcolor >> 8) & 0xff) / 255,
            (GLfloat)(fcolor & 0xff) / 255,
            1.0f};

        gllc_polyline_build(pline->ver, pline->ver_size, pline->DE_bound, pline->DE_fill, color_, fcolor_, pline->closed, pline->filled);

        ent->modified = 0;
}

struct gllc_polyline *gllc_polyline_create(struct gllc_block *block, int closed, int filled)
{
        struct gllc_polyline *ent = malloc(sizeof(struct gllc_polyline));
        if (ent)
        {
                memset(ent, 0, sizeof(struct gllc_polyline));

                ent->__ent.__obj.prop_def = g_props_def;
                ent->__ent.destroy = destruct;
                ent->__ent.build = build;
                ent->__ent.block = block;
                ent->closed = closed;
                ent->filled = filled;
                ent->__ent.props.color = -1;
                ent->__ent.props.fcolor = -1;
                ent->__ent.modified = 1;
        }

        return ent;
_error:
        if (ent->DE_bound)
                gllc_DE_destroy(ent->DE_bound);

        free(ent);

        return 0;
}

static int push_ver(struct gllc_polyline *pline, double x, double y)
{
        if (pline->ver_size + 2 > pline->ver_cap)
        {
                size_t new_cap = pline->ver_cap ? pline->ver_cap * 2 : 8;
                struct gllc_polyline_vertex *new_ver = (struct gllc_polyline_vertex *)realloc(pline->ver, sizeof(struct gllc_polyline_vertex) * new_cap);
                if (!new_ver)
                {
                        return 0;
                }
                pline->ver = new_ver;
                pline->ver_cap = new_cap;
        }

        pline->ver[pline->ver_size].x = x;
        pline->ver[pline->ver_size].y = y;
        pline->ver_size++;

        return 1;
}

void gllc_polyline_add_ver(struct gllc_polyline *pline, double x, double y)
{
        push_ver(pline, x, y);

        pline->__ent.modified = 1;
}