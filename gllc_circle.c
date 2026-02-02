#include "gllc_circle.h"
#include "gllc_block.h"
#include "gllc_block_entity.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static const struct gllc_prop_def g_internal_props_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_block_entity_prop_def,
                                                    g_internal_props_def,
                                                    0};

#define PI 3.14159265358979323846
#define N 64

void gllc_circle_build(double x, double y, double radius, GLfloat *color, GLfloat *fcolor, struct gllc_DE *DE_bound, struct gllc_DE *DE_fill, int filled)
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

        struct gllc_DE_config DE_conf = {};
        DE_conf.V = V,
        DE_conf.I = I,
        DE_conf.color = color,
        DE_conf.V_count = N,
        DE_conf.I_count = N;

        gllc_DE_update(DE_bound, &DE_conf);

        if (filled)
        {
                GLfloat V_fill[2 * (N + 1)];
                GLuint I_fill[3 * N];

                V_fill[0] = (GLfloat)x;
                V_fill[1] = (GLfloat)y;

                memcpy(&V_fill[2], V, 2 * N * sizeof(GLfloat));

                int idx = 0;
                for (int i = 1; i < N; i++)
                {
                        I_fill[idx++] = 0;
                        I_fill[idx++] = i;
                        I_fill[idx++] = i + 1;
                }
                I_fill[idx++] = 0;
                I_fill[idx++] = N;
                I_fill[idx++] = 1;

                struct gllc_DE_config DE_conf_fill = {0};
                DE_conf_fill.V = V_fill;
                DE_conf_fill.I = I_fill;
                DE_conf_fill.color = fcolor;
                DE_conf_fill.V_count = N + 1;
                DE_conf_fill.I_count = 3 * N;

                gllc_DE_update(DE_fill, &DE_conf_fill);
        }
}

static void build(struct gllc_block_entity *ent, struct gllc_DBD *DBD)
{
        struct gllc_circle *circle = (struct gllc_circle *)ent;

        if (GLLC_ENT_FLAG(ent, GLLC_ENT_FILLED))
        {
                circle->DE_fill = gllc_DE_create(DBD, GL_TRIANGLE_FAN);
                if (!circle->DE_fill)
                        return;
        }
        else
        {
                if (circle->DE_fill)
                        gllc_DE_destroy(circle->DE_fill);
                circle->DE_fill = NULL;
        }
        if (!circle->DE_bound)
        {
                circle->DE_bound = gllc_DE_create(DBD, GL_LINE_LOOP);
                if (!circle->DE_bound)
                        return;
        }

        GLfloat color_[4], fcolor_[4];
        gllc_ent_color_4f(gllc_ent_color(ent), color_);
        gllc_ent_color_4f(gllc_ent_fcolor(ent), fcolor_);

        gllc_circle_build(
            circle->x,
            circle->y,
            circle->radius,
            color_,
            fcolor_,
            circle->DE_bound,
            circle->DE_fill,
            GLLC_ENT_FLAG(ent, GLLC_ENT_FILLED));

        GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_MODIFIED);
}

static void destruct(struct gllc_block_entity *ent)
{
        struct gllc_circle *circle = (struct gllc_circle *)ent;

        if (circle->DE_bound)
                gllc_DE_destroy(circle->DE_bound);

        if (circle->DE_fill)
                gllc_DE_destroy(circle->DE_fill);
}

static int bbox(struct gllc_block_entity *ent, double *bbox_x0, double *bbox_y0, double *bbox_x1, double *bbox_y1)
{
        struct gllc_circle *circle = (struct gllc_circle *)ent;

        *bbox_x0 = circle->x - circle->radius;
        *bbox_y0 = circle->y - circle->radius;
        *bbox_x1 = circle->x + circle->radius;
        *bbox_y1 = circle->y + circle->radius;

        return 1;
}

static int picked(struct gllc_block_entity *ent, double x, double y)
{
        struct gllc_circle *c = (struct gllc_circle *)ent;

        return 1;
}

static int selected(struct gllc_block_entity *ent, double x0, double y0, double x1, double y1)
{
        struct gllc_circle *c = (struct gllc_circle *)ent;

        return 0;
}

const static struct gllc_block_entity_vtable g_vtable = {
    .build = build,
    .destroy = destruct,
    .bbox = bbox,
    .picked = picked,
    .selected = selected,
    .type = GLLC_ENT_CIRCLE};

struct gllc_circle *gllc_circle_create(struct gllc_block *block, double x, double y, double radius, int filled)
{
        struct gllc_circle *ent = malloc(sizeof(struct gllc_circle));
        if (ent)
        {
                memset(ent, 0, sizeof(struct gllc_circle));

                GLLC_ENT_INIT(ent, g_props_def, block, &g_vtable);

                ent->x = x;
                ent->y = y;
                ent->radius = radius;

                if (filled)
                {
                        GLLC_ENT_SET_FLAG(ent, GLLC_ENT_FILLED);
                }
        }

        return ent;
}