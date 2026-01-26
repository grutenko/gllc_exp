#include "gllc_rect.h"
#include "gllc_draw_buffer.h"
#include "include/gllc_block.h"
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

void gllc_rect_build(double x0, double y0, double x1, double y1, GLfloat *color, struct gllc_DE *DE_bound)
{
        if (x0 > x1)
                _swap(&x0, &x1);
        if (y0 > y1)
                _swap(&y0, &y1);

        GLfloat V[] = {
            (GLfloat)x0, (GLfloat)y0,
            (GLfloat)x1, (GLfloat)y0,
            (GLfloat)x1, (GLfloat)y1,
            (GLfloat)x0, (GLfloat)y1};
            
        GLuint I[] = {0, 1, 2, 3};

        struct gllc_DE_config DE_conf = {
            .skip = NULL,
            .v = V,
            .i = I,
            .color = color,
            .v_count = 4,
            .i_count = 4};

        gllc_DE_update(DE_bound, &DE_conf);
}

static void build(struct gllc_block_entity *ent, struct gllc_DBD_batch *DBD_batch)
{
        int color = gllc_block_entity_color(ent);
        int fcolor = gllc_block_entity_fcolor(ent);

        GLfloat color_[] = {
            (GLfloat)((color >> 16) & 0xff) / 255,
            (GLfloat)((color >> 8) & 0xff) / 255,
            (GLfloat)(color & 0xff) / 255,
            1.0f};

        gllc_rect_build(
            ((struct gllc_rect *)ent)->x0,
            ((struct gllc_rect *)ent)->y0,
            ((struct gllc_rect *)ent)->x1,
            ((struct gllc_rect *)ent)->y1,
            color_,
            ((struct gllc_rect *)ent)->DE_bound);

        ent->modified = 0;
}

static void destruct(struct gllc_block_entity *ent)
{
        struct gllc_rect *rect = (struct gllc_rect *)ent;
        if (rect->DE_bound)
                gllc_DE_destroy(rect->DE_bound);
}

struct gllc_rect *gllc_rect_create(struct gllc_block *block, double x0, double y0, double x1, double y1)
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

                ent->DE_bound = gllc_DE_create(&block->DBD_batch.GL_line_loop);

                if (!ent->DE_bound)
                        goto _error;

                ent->x0 = x0;
                ent->y0 = y0;
                ent->x1 = x1;
                ent->y1 = y1;
        }
        return ent;
_error:
        if (ent->DE_bound)
                gllc_DE_destroy(ent->DE_bound);
        free(ent);

        return 0;
}