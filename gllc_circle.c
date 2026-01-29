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

        struct gllc_DE_config DE_conf = {
            .V = V,
            .I = I,
            .color = color,
            .V_count = N,
            .I_count = N};

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

                struct gllc_DE_config DE_conf_fill = {
                    .V = V_fill,
                    .I = I_fill,
                    .color = fcolor,
                    .V_count = N + 1,
                    .I_count = 3 * N};

                gllc_DE_update(DE_fill, &DE_conf_fill);
        }
}

static void build(struct gllc_block_entity *ent, struct gllc_DBD *DBD)
{
        struct gllc_circle *circle = (struct gllc_circle *)ent;

        if (circle->filled)
        {
                circle->DE_fill = gllc_DE_create(DBD, GL_TRIANGLE_FAN);

                if (!circle->DE_fill)
                {
                        return;
                }
        }
        else
        {
                if (circle->DE_fill)
                {
                        gllc_DE_destroy(circle->DE_fill);
                }

                circle->DE_fill = NULL;
        }

        if (!circle->DE_bound)
        {
                circle->DE_bound = gllc_DE_create(DBD, GL_LINE_LOOP);

                if (!circle->DE_bound)
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

        gllc_circle_build(
            circle->x,
            circle->y,
            circle->radius,
            color_,
            fcolor_,
            circle->DE_bound,
            circle->DE_fill,
            circle->filled);

        ent->modified = 0;
}

static void destruct(struct gllc_block_entity *ent)
{
        struct gllc_circle *circle = (struct gllc_circle *)ent;

        if (circle->DE_bound)
        {
                gllc_DE_destroy(circle->DE_bound);
        }
}

struct gllc_circle *gllc_circle_create(struct gllc_block *block, double x, double y, double radius, int filled)
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

                ent->x = x;
                ent->y = y;
                ent->radius = radius;
                ent->filled = filled;
        }
        return ent;
_error:
        if (ent->DE_bound)
        {
                gllc_DE_destroy(ent->DE_bound);
        }
        if (ent->DE_fill)
        {
                gllc_DE_destroy(ent->DE_fill);
        }
        free(ent);

        return 0;
}