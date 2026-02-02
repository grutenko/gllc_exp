#include "gllc_polyline.h"
#include "gllc_block.h"
#include "gllc_block_entity.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"

#include "tess2/tess2.h"

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
        if (pline->DE_fill)
                gllc_DE_destroy(pline->DE_fill);

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

        struct gllc_DE_config DE_config = {0};
        DE_config.V = G_vertices;
        DE_config.I = G_indices;
        DE_config.V_count = V_count;
        DE_config.I_count = V_count;
        DE_config.color = color;

        gllc_DE_update(DE_bound, &DE_config);

        if (closed && filled)
        {
                TESStesselator *tess = tessNewTess(NULL);
                if (tess)
                {
                        tessAddContour(tess, 2, G_vertices, sizeof(GLfloat) * 2, V_count);

                        if (tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, 3, 2, NULL))
                        {
                                const float *verts = tessGetVertices(tess);
                                int vert_count = tessGetVertexCount(tess);
                                int indices_count = tessGetElementCount(tess);
                                const int *indices = tessGetElements(tess);

                                struct gllc_DE_config DE_config = {0};
                                DE_config.V = verts;
                                DE_config.I = (GLuint *)indices;
                                DE_config.V_count = vert_count;
                                DE_config.I_count = indices_count * 3;
                                DE_config.color = fcolor;

                                gllc_DE_update(DE_fill, &DE_config);
                        }

                        tessDeleteTess(tess);
                }
        }
}

static void build(struct gllc_block_entity *ent, struct gllc_DBD *DBD)
{
        struct gllc_polyline *pline = (struct gllc_polyline *)ent;

        if (GLLC_ENT_FLAG(ent, GLLC_ENT_CLOSED) && GLLC_ENT_FLAG(ent, GLLC_ENT_FILLED))
        {
                pline->DE_fill = gllc_DE_create(DBD, GL_TRIANGLES);
                if (!pline->DE_fill)
                        return;
        }
        else
        {
                if (pline->DE_fill)
                        gllc_DE_destroy(pline->DE_fill);
                pline->DE_fill = NULL;
        }
        if (!pline->DE_bound)
        {
                if (GLLC_ENT_FLAG(ent, GLLC_ENT_CLOSED))
                {
                        pline->DE_bound = gllc_DE_create(DBD, GL_LINE_LOOP);
                        if (!pline->DE_bound)
                                return;
                }
                else
                {
                        pline->DE_bound = gllc_DE_create(DBD, GL_LINE_STRIP);
                        if (!pline->DE_bound)
                                return;
                }
        }

        if (GLLC_ENT_FLAG(ent, GLLC_ENT_CLOSED))
        {
                pline->DE_bound->GL_type = GL_LINE_LOOP;
        }
        else
        {
                pline->DE_bound->GL_type = GL_LINE_STRIP;
        }

        GLfloat color_[4], fcolor_[4];
        gllc_ent_color_4f(gllc_ent_color(ent), color_);
        gllc_ent_color_4f(gllc_ent_fcolor(ent), fcolor_);

        gllc_polyline_build(
            pline->ver,
            pline->ver_size,
            pline->DE_bound,
            pline->DE_fill,
            color_,
            fcolor_,
            GLLC_ENT_FLAG(ent, GLLC_ENT_CLOSED),
            GLLC_ENT_FLAG(ent, GLLC_ENT_FILLED));

        GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_MODIFIED);
}

static int bbox(struct gllc_block_entity *ent, double *bbox_x0, double *bbox_y0, double *bbox_x1, double *bbox_y1)
{
        struct gllc_polyline *pline = (struct gllc_polyline *)ent;

        if (pline->ver_size == 0)
                return 0;

        double minx = pline->ver[0].x;
        double miny = pline->ver[0].y;
        double maxx = minx;
        double maxy = miny;

        for (int i = 1; i < pline->ver_size; i++)
        {
                double x = pline->ver[i].x;
                double y = pline->ver[i].y;

                if (x < minx)
                        minx = x;
                if (y < miny)
                        miny = y;
                if (x > maxx)
                        maxx = x;
                if (y > maxy)
                        maxy = y;
        }

        *bbox_x0 = minx;
        *bbox_y0 = miny;
        *bbox_x1 = maxx;
        *bbox_y1 = maxy;

        return 1;
}

static int picked(struct gllc_block_entity *ent, double x, double y)
{
        struct gllc_polyline *pl = (struct gllc_polyline *)ent;

        int cnt = 0;
        int i;
        for (i = 0; i < pl->ver_size - 1; i++)
        {
                double x1 = pl->ver[i].x, y1 = pl->ver[i].y;
                double x2 = pl->ver[i + 1].x, y2 = pl->ver[i + 1].y;

                if ((y1 > y) != (y2 > y))
                {
                        double xint = x1 + (y - y1) * (x2 - x1) / (y2 - y1);
                        if (xint > x)
                                cnt++;
                }
        }

        return cnt % 2 == 1;
}

static int selected(struct gllc_block_entity *ent, double x0, double y0, double x1, double y1)
{
        struct gllc_polyline *c = (struct gllc_polyline *)ent;

        double bbox_x0, bbox_y0, bbox_x1, bbox_y1;
        int ok = bbox(ent, &bbox_x0, &bbox_y0, &bbox_x1, &bbox_y1);

        return ok && bbox_x0 >= x0 && bbox_y0 >= y0 && bbox_x1 <= x1 && bbox_y1 <= y1;
}

const static struct gllc_block_entity_vtable g_vtable = {
    .build = build,
    .destroy = destruct,
    .bbox = bbox,
    .picked = picked,
    .selected = selected};

struct gllc_polyline *gllc_polyline_create(struct gllc_block *block, int closed, int filled)
{
        struct gllc_polyline *ent = malloc(sizeof(struct gllc_polyline));
        if (ent)
        {
                memset(ent, 0, sizeof(struct gllc_polyline));

                GLLC_ENT_INIT(ent, g_props_def, block, &g_vtable);

                if (closed)
                {
                        GLLC_ENT_SET_FLAG(ent, GLLC_ENT_CLOSED);
                }

                if (filled)
                {
                        GLLC_ENT_SET_FLAG(ent, GLLC_ENT_FILLED);
                }
        }

        return ent;
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

        memset(&pline->ver[pline->ver_size], 0, sizeof(struct gllc_polyline_vertex));
        // TODO: обявление свойств для VERTEX
        pline->ver[pline->ver_size].__obj.prop_def = 0;
        pline->ver[pline->ver_size].x = x;
        pline->ver[pline->ver_size].y = y;
        pline->ver_size++;

        return 1;
}

void gllc_polyline_add_ver(struct gllc_polyline *pline, double x, double y)
{
        push_ver(pline, x, y);

        GLLC_ENT_SET_FLAG(pline, GLLC_ENT_MODIFIED);
}