#include "gllc_vertex.h"
#include "gllc_block_entity.h"
#include "gllc_draw_buffer.h"

#include <stdlib.h>
#include <string.h>

static const struct gllc_prop_def g_internal_props_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_block_entity_prop_def,
                                                    g_internal_props_def,
                                                    0};

static void _build(struct gllc_block_entity *ent, struct gllc_DBD *DBD)
{
        struct gllc_vertex *vertex = (struct gllc_vertex *)ent;

        if (!vertex->DE_fill)
        {
                vertex->DE_fill = gllc_DE_create(DBD, GL_TRIANGLES);
                if (!vertex->DE_fill)
                        return;
        }

        GLfloat V[] = {
            (GLfloat)vertex->x - 3.0f,
            (GLfloat)vertex->y - 3.0f,
            (GLfloat)vertex->x + 3.0f,
            (GLfloat)vertex->y - 3.0f,
            (GLfloat)vertex->x + 3.0f,
            (GLfloat)vertex->y + 3.0f,
            (GLfloat)vertex->x - 3.0f,
            (GLfloat)vertex->y + 3.0f,
        };
        GLuint I[] = {0, 1, 2, 0, 2, 3};
        GLfloat color[] = {0.0f, 0.0f, 0.0f, 1.0f};
        GLuint flags = GLLC_POINT_SCALE_INVARIANT;
        GLfloat center_point[] = {(GLfloat)vertex->x,
                                  (GLfloat)vertex->y};

        struct gllc_DE_config conf = {
            .center_point = center_point,
            .color = color,
            .flags = &flags,
            .V = V,
            .I = I,
            .V_count = 4,
            .I_count = 6};

        gllc_DE_update(vertex->DE_fill, &conf);
}

static int _picked(struct gllc_block_entity *ent, double scale, double x, double y)
{
        return 0;
}

static int _selected(struct gllc_block_entity *ent, double scale, double x0, double y0, double x1, double y1)
{
        return 0;
}

static int _bbox(struct gllc_block_entity *ent, double scale, double *bbox_x0, double *bbox_y0, double *bbox_x1, double *bbox_y1)
{
        return 0;
}

static void _destroy(struct gllc_block_entity *ent)
{
        if (((struct gllc_vertex *)ent)->DE_fill)
                gllc_DE_destroy(((struct gllc_vertex *)ent)->DE_fill);
}

static int _vertices(struct gllc_block_entity *ent, double scale, double *ver)
{
        // Не используется для этого примитива
        return 0;
}

static const struct gllc_block_entity_vtable g_vtable = {
    .build = _build,
    .destroy = _destroy,
    .bbox = _bbox,
    .picked = _picked,
    .selected = _selected,
    .vertices = _vertices,
    .type = GLLC_ENT_VERTEX};

struct gllc_vertex *gllc_vertex_create(struct gllc_block *block, struct gllc_block_entity *owner, double x, double y, int index)
{
        struct gllc_vertex *vertex = malloc(sizeof(struct gllc_vertex));
        if (vertex)
        {
                memset(vertex, 0, sizeof(struct gllc_vertex));
                GLLC_ENT_INIT(vertex, g_props_def, block, &g_vtable);

                vertex->x = x;
                vertex->y = y;
                vertex->index = index;
                vertex->owner = owner;

                //GLLC_ENT_SET_FLAG(vertex, GLLC_ENT_SCREEN_SIZE);
        }
        return vertex;
}