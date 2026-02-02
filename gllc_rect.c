#include "gllc_rect.h"
#include "gllc_block_entity.h"
#include "gllc_draw_buffer.h"
#include "include/gllc_block.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static const struct gllc_prop_def g_internal_props_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_block_entity_prop_def,
                                                    g_internal_props_def,
                                                    0};

#define PI 3.14159265358979323846

void gllc_rect_build(double x, double y, double width, double height, double angle, GLfloat *color, GLfloat *fcolor, struct gllc_DE *DE_bound, struct gllc_DE *DE_fill, int filled)
{
        double rad = angle * (PI / 180.0);
        GLfloat x0, y0, x1, y1;

        x0 = (GLfloat)x;
        y0 = (GLfloat)y;
        x1 = (GLfloat)(x + width);
        y1 = (GLfloat)(y + height);

        GLfloat V[] = {
            x0, y0,
            x1, y0,
            x1, y1,
            x0, y1};

        GLuint I[] = {0, 1, 2, 3};

        struct gllc_DE_config DE_conf = {0};
        DE_conf.V = V;
        DE_conf.I = I;
        DE_conf.color = color;
        DE_conf.V_count = 4;
        DE_conf.I_count = 4;

        gllc_DE_update(DE_bound, &DE_conf);

        if (filled)
        {
                GLuint I_fill[] = {
                    0, 1, 2,
                    0, 2, 3};

                struct gllc_DE_config DE_conf = {0};
                DE_conf.V = V;
                DE_conf.I = I_fill;
                DE_conf.color = fcolor;
                DE_conf.V_count = 4;
                DE_conf.I_count = 6;

                gllc_DE_update(DE_fill, &DE_conf);
        }
}

static void build(struct gllc_block_entity *ent, struct gllc_DBD *DBD)
{
        struct gllc_rect *rect = (struct gllc_rect *)ent;

        if (GLLC_ENT_FLAG(ent, GLLC_ENT_FILLED))
        {
                rect->DE_fill = gllc_DE_create(DBD, GL_TRIANGLES);
                if (!rect->DE_fill)
                        return;
        }
        else
        {
                if (rect->DE_fill)
                        gllc_DE_destroy(rect->DE_fill);
                rect->DE_fill = NULL;
        }

        if (!rect->DE_bound)
        {
                rect->DE_bound = gllc_DE_create(DBD, GL_LINE_LOOP);
                if (!rect->DE_bound)
                        return;
        }

        GLfloat color_[4], fcolor_[4];
        gllc_ent_color_4f(gllc_ent_color(ent), color_);
        gllc_ent_color_4f(gllc_ent_fcolor(ent), fcolor_);

        gllc_rect_build(
            rect->x,
            rect->y,
            rect->width,
            rect->height,
            rect->angle,
            color_,
            fcolor_,
            rect->DE_bound,
            rect->DE_fill,
            GLLC_ENT_FLAG(ent, GLLC_ENT_FILLED));

        GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_MODIFIED);
}

static void destruct(struct gllc_block_entity *ent)
{
        struct gllc_rect *rect = (struct gllc_rect *)ent;
        if (rect->DE_bound)
                gllc_DE_destroy(rect->DE_bound);
        if (rect->DE_fill)
                gllc_DE_destroy(rect->DE_fill);
}

static int bbox(struct gllc_block_entity *ent, double *bbox_x0, double *bbox_y0, double *bbox_x1, double *bbox_y1)
{
        return 1;
}

static int picked(struct gllc_block_entity *ent, double x0, double y0, double x1, double y1)
{
        struct gllc_rect *c = (struct gllc_rect *)ent;

        return 1;
}

const static struct gllc_block_entity_vtable g_vtable = {
    .build = build,
    .destroy = destruct,
    .bbox = bbox};

struct gllc_rect *gllc_rect_create(struct gllc_block *block, double x, double y, double width, double height, double angle, int filled)
{
        struct gllc_rect *ent = malloc(sizeof(struct gllc_rect));
        if (ent)
        {
                memset(ent, 0, sizeof(struct gllc_rect));

                GLLC_ENT_INIT(ent, g_props_def, block, &g_vtable);

                ent->x = x;
                ent->y = y;
                ent->width = width;
                ent->height = height;
                ent->angle = angle;

                if (filled)
                {
                        GLLC_ENT_SET_FLAG(ent, GLLC_ENT_FILLED);
                }
        }

        return ent;
}