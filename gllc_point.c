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
                {
                        return;
                }
        }

        int color = gllc_block_entity_color(ent);

        GLfloat color_[] = {
            (GLfloat)((color >> 16) & 0xff) / 255,
            (GLfloat)((color >> 8) & 0xff) / 255,
            (GLfloat)(color & 0xff) / 255,
            1.0f};

        GLfloat V[] = {
            (GLfloat)point->x - 5.0f, (GLfloat)point->y,
            (GLfloat)point->x + 5.0f, (GLfloat)point->y,
            (GLfloat)point->x, (GLfloat)point->y - 5.0f,
            (GLfloat)point->x, (GLfloat)point->y + 5.0f};
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

        ent->modified = 0;
}

static void destruct(struct gllc_block_entity *ent)
{
        struct gllc_point *point = (struct gllc_point *)ent;
        if (point->DE)
                gllc_DE_destroy(point->DE);
}

struct gllc_point *gllc_point_create(struct gllc_block *block, double x, double y)
{
        struct gllc_point *ent = malloc(sizeof(struct gllc_point));
        if (ent)
        {
                memset(ent, 0, sizeof(struct gllc_point));

                ent->__ent.__obj.prop_def = g_props_def;
                ent->__ent.destroy = destruct;
                ent->__ent.build = build;
                ent->__ent.block = block;
                ent->__ent.modified = 1;

                ent->x = x;
                ent->y = y;
        }

        return ent;

_error:
        free(ent);
        return 0;
}