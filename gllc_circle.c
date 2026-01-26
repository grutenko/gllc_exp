#include "gllc_circle.h"
#include "gllc_block.h"
#include "gllc_block_entity.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static const struct gllc_prop_def g_internal_props_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_block_entity_prop_def,
                                                    g_internal_props_def,
                                                    0};

#define PI 3.14159265358979323846
#define N 56

void gllc_circle_build(double x, double y, double radius, GLfloat *color, struct gllc_DE *DE_bound)
{
        GLfloat V[2 * N];
        GLuint I[N];

        int i;
        for (i = 0; i < N; i++)
        {
#define THETA(index) (2.0 * PI * (index) / N)
                V[i * 2] = x + radius * cos(THETA(i));
                V[i * 2 + 1] = y + radius * sin(THETA(i));
                I[i] = i;
        }

        struct gllc_DE_config DE_conf = {
            .skip = NULL,
            .v = V,
            .i = I,
            .color = color,
            .v_count = N,
            .i_count = N};

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

        gllc_circle_build(
            ((struct gllc_circle *)ent)->x,
            ((struct gllc_circle *)ent)->y,
            ((struct gllc_circle *)ent)->radius,
            color_,
            ((struct gllc_circle *)ent)->DE_bound);

        ent->modified = 0;
}

static void destruct(struct gllc_block_entity *ent)
{
        struct gllc_circle *circle = (struct gllc_circle *)ent;
        if (circle->DE_bound)
                gllc_DE_destroy(circle->DE_bound);
}

struct gllc_circle *gllc_circle_create(struct gllc_block *block, double x, double y, double radius)
{
        struct gllc_circle *ent = malloc(sizeof(struct gllc_circle));
        if (ent)
        {
                memset(ent, 0, sizeof(struct gllc_circle));

                ent->__ent.__obj.prop_def = g_props_def;
                ent->__ent.destroy = destruct;
                ent->__ent.build = build;
                ent->__ent.block = block;
                ent->__ent.modified = 1;

                ent->DE_bound = gllc_DE_create(&block->DBD_batch.GL_line_loop);

                if (!ent->DE_bound)
                        goto _error;

                ent->x = x;
                ent->y = y;
                ent->radius = radius;
        }
        return ent;
_error:
        if (ent->DE_bound)
                gllc_DE_destroy(ent->DE_bound);
        free(ent);

        return 0;
}