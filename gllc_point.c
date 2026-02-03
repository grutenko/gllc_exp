#include "gllc_point.h"
#include "glad.h"
#include "gllc_block_entity.h"
#include "gllc_draw_buffer.h"

#include <stdlib.h>
#include <string.h>

static struct gllc_prop_def g_internal_props_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_block_entity_prop_def,
                                                    g_internal_props_def,
                                                    0};

static void build(struct gllc_block_entity *ent, struct gllc_DBD *DBD)
{
        struct gllc_point *point = (struct gllc_point *)ent;

        if (!point->DE)
        {
                point->DE = gllc_DE_create(DBD, GL_LINES);
                if (!point->DE)
                        return;
        }

        GLfloat color_[4];
        gllc_ent_color_4f(gllc_ent_color(ent), color_);

        GLfloat V[] = {
            (GLfloat)point->x - 3.0f, (GLfloat)point->y,
            (GLfloat)point->x + 3.0f, (GLfloat)point->y,
            (GLfloat)point->x, (GLfloat)point->y - 3.0f,
            (GLfloat)point->x, (GLfloat)point->y + 3.0f};
        GLuint I[] = {0, 1, 2, 3};

        GLuint flags = GLLC_POINT_SCALE_INVARIANT;

        GLfloat center[] = {(GLfloat)point->x, (GLfloat)point->y};

        struct gllc_DE_config DE_conf =
            {
                .V = V,
                .I = I,
                .V_count = 4,
                .I_count = 4,
                .color = color_,
                .flags = &flags,
                .center_point = center};

        gllc_DE_update(point->DE, &DE_conf);
}

static void destruct(struct gllc_block_entity *ent)
{
        struct gllc_point *point = (struct gllc_point *)ent;
        if (point->DE)
                gllc_DE_destroy(point->DE);
}

static int bbox(struct gllc_block_entity *ent, double *bbox_x0, double *bbox_y0, double *bbox_x1, double *bbox_y1)
{
        struct gllc_point *point = (struct gllc_point *)ent;

        *bbox_x0 = point->x;
        *bbox_y0 = point->y;
        *bbox_x1 = point->x;
        *bbox_y1 = point->y;

        return 1;
}

static int picked(struct gllc_block_entity *ent, double x, double y)
{
        return 0;
}

static int selected(struct gllc_block_entity *ent, double x0, double y0, double x1, double y1)
{
        struct gllc_point *p = (struct gllc_point *)ent;

        return x0 <= p->x && y0 <= p->y && x1 >= p->x && y1 >= p->y;
}

static int vertices(struct gllc_block_entity *ent, double *ver)
{
        return 0;
}

const static struct gllc_block_entity_vtable g_vtable = {
    .build = build,
    .destroy = destruct,
    .bbox = bbox,
    .picked = picked,
    .selected = selected,
    .vertices = vertices,
    .type = GLLC_ENT_POINT};

struct gllc_point *gllc_point_create(struct gllc_block *block, double x, double y)
{
        struct gllc_point *ent = malloc(sizeof(struct gllc_point));
        if (ent)
        {
                memset(ent, 0, sizeof(struct gllc_point));

                GLLC_ENT_INIT(ent, g_props_def, block, &g_vtable);

                ent->x = x;
                ent->y = y;
        }

        return ent;

_error:
        free(ent);
        return 0;
}