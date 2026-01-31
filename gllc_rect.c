#include "gllc_rect.h"
#include "gllc_draw_buffer.h"
#include "include/gllc_block.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static const struct gllc_prop_def g_internal_props_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_block_entity_prop_def,
                                                    g_internal_props_def,
                                                    0};

static inline void _swap(double *a, double *b)
{
        double t = *a;
        *a = *b;
        *b = t;
}

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

        struct gllc_DE_config DE_conf = {
            .V = V,
            .I = I,
            .color = color,
            .V_count = 4,
            .I_count = 4};

        gllc_DE_update(DE_bound, &DE_conf);

        if (filled)
        {
                GLuint I_fill[] = {
                    0, 1, 2,
                    0, 2, 3};

                struct gllc_DE_config DE_conf = {
                    .V = V,
                    .I = I_fill,
                    .color = fcolor,
                    .V_count = 4,
                    .I_count = 6};

                gllc_DE_update(DE_fill, &DE_conf);
        }
}

static void build(struct gllc_block_entity *ent, struct gllc_DBD *DBD)
{
        struct gllc_rect *rect = (struct gllc_rect *)ent;

        if (rect->filled)
        {
                rect->DE_fill = gllc_DE_create(DBD, GL_TRIANGLES);

                if (!rect->DE_fill)
                {
                        return;
                }
        }
        else
        {
                if (rect->DE_fill)
                {
                        gllc_DE_destroy(rect->DE_fill);
                }

                rect->DE_fill = NULL;
        }

        if (!rect->DE_bound)
        {
                rect->DE_bound = gllc_DE_create(DBD, GL_LINE_LOOP);

                if (!rect->DE_bound)
                {
                        return;
                }
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
            rect->filled);

        ent->modified = 0;
}

static void destruct(struct gllc_block_entity *ent)
{
        struct gllc_rect *rect = (struct gllc_rect *)ent;
        if (rect->DE_bound)
                gllc_DE_destroy(rect->DE_bound);
}

struct gllc_rect *gllc_rect_create(struct gllc_block *block, double x, double y, double width, double height, double angle, int filled)
{
        struct gllc_rect *ent = malloc(sizeof(struct gllc_rect));
        if (ent)
        {
                memset(ent, 0, sizeof(struct gllc_rect));

                ent->__ent.__obj.prop_def = g_props_def;
                ent->__ent.destroy = destruct;
                ent->__ent.build = build;
                ent->__ent.block = block;
                ent->__ent.modified = 1;

                ent->x = x;
                ent->y = y;
                ent->width = width;
                ent->height = height;
                ent->angle = angle;
                ent->filled = filled;
        }
        return ent;
_error:
        if (ent->DE_bound)
                gllc_DE_destroy(ent->DE_bound);
        free(ent);

        return 0;
}