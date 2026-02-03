#include "gllc_block_entity.h"
#include "gllc_block.h"
#include "gllc_draw_buffer.h"
#include "gllc_layer.h"
#include "gllc_object.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>

const struct gllc_prop_def g_block_entity_prop_def[] = {};

int gllc_ent_color(struct gllc_block_entity *ent)
{
        if (ent->props.color >= 0)
                return ent->props.color;
        if (ent->layer && ent->layer->props.color >= 0)
                return ent->layer->props.color;
        if (ent->block->props.color >= 0)
                return ent->block->props.color;
        return 0;
}

int gllc_ent_fcolor(struct gllc_block_entity *ent)
{
        if (ent->props.fcolor >= 0)
                return ent->props.fcolor;
        if (ent->layer && ent->layer->props.fcolor >= 0)
                return ent->layer->props.fcolor;
        if (ent->block->props.fcolor >= 0)
                return ent->block->props.fcolor;
        return 0;
}

inline void gllc_ent_color_4f(int color, float *out_)
{
        out_[0] = (GLfloat)((color >> 16) & 0xff) / 255;
        out_[1] = (GLfloat)((color >> 8) & 0xff) / 255;
        out_[2] = (GLfloat)(color & 0xff) / 255;
        out_[3] = 1.0f;
}

void gllc_ent_set_color(struct gllc_block_entity *ent, int color)
{
        ent->props.color = color;

        GLLC_ENT_SET_FLAG(ent, GLLC_ENT_MODIFIED);
}

void gllc_ent_set_fcolor(struct gllc_block_entity *ent, int fcolor)
{
        ent->props.fcolor = fcolor;

        GLLC_ENT_SET_FLAG(ent, GLLC_ENT_MODIFIED);
}

void gllc_ent_set_layer(struct gllc_block_entity *ent, struct gllc_layer *layer)
{
        ent->layer = layer;
        if (ent->props.color == -1 || ent->props.fcolor == -1)
                GLLC_ENT_SET_FLAG(ent, GLLC_ENT_MODIFIED);
}

void gllc_ent_destroy(struct gllc_block_entity *ent)
{
        int i;
        if (ent->vtable->destroy)
                ent->vtable->destroy(ent);
        for (i = 0; i < ent->DE_verices_count; i++)
                gllc_DE_destroy(ent->DE_vertices[i]);
        free(ent->DE_vertices);
        free(ent);
}

struct gllc_block_entity *gllc_ent_get_next(struct gllc_block_entity *ent)
{
        if (!ent)
                return NULL;

        return ent->next;
}

void build_vertices(struct gllc_block_entity *ent, const double *ver, size_t ver_count)
{
        int i;

        if (ent->DE_vertices_cap < ver_count)
        {
                int new_DE_cap = ver_count;
                struct gllc_DE **new_DE = realloc(ent->DE_vertices, sizeof(struct gllc_DE *) * new_DE_cap);
                if (!new_DE)
                        return;
                ent->DE_vertices = new_DE;
                ent->DE_vertices_cap = new_DE_cap;
        }

        if (ver_count < ent->DE_verices_count)
        {
                for (i = ver_count; i < ent->DE_verices_count; i++)
                {
                        gllc_DE_destroy(ent->DE_vertices[i]);
                }
        }
        else if (ver_count > ent->DE_verices_count)
        {
                for (i = ent->DE_verices_count; i < ver_count; i++)
                {
                        ent->DE_vertices[i] = gllc_DE_create(&ent->block->I_DBD, GL_TRIANGLES);
                        if (!ent->DE_vertices[i])
                        {
                                printf("Cannot create DE\n");
                                return;
                        }
                }
        }

        ent->DE_verices_count = ver_count;

        for (i = 0; i < ver_count; i++)
        {
                struct gllc_DE_config conf;

                GLfloat V[] = {
                    (GLfloat)ver[i * 2] - 3.0f,
                    (GLfloat)ver[i * 2 + 1] - 3.0f,
                    (GLfloat)ver[i * 2] + 3.0f,
                    (GLfloat)ver[i * 2 + 1] - 3.0f,
                    (GLfloat)ver[i * 2] + 3.0f,
                    (GLfloat)ver[i * 2 + 1] + 3.0f,
                    (GLfloat)ver[i * 2] - 3.0f,
                    (GLfloat)ver[i * 2 + 1] + 3.0f,
                };
                GLuint I[] = {0, 1, 2, 0, 2, 3};
                GLuint flags = GLLC_POINT_SCALE_INVARIANT;
                GLfloat color[] = {0.0f, 0.0f, 0.0f, 1.0f};
                GLfloat center_point[] = {(GLfloat)ver[i * 2], (GLfloat)ver[i * 2 + 1]};

                conf.center_point = center_point;
                conf.V = V;
                conf.I = I;
                conf.V_count = 4;
                conf.I_count = 6;
                conf.flags = &flags;
                conf.color = color;
                gllc_DE_update(ent->DE_vertices[i], &conf);
        }
}

void gllc_ent_build(struct gllc_block_entity *ent)
{
        int i;
        size_t ver_count;
        double *ver = 0ULL;
        double ver_stack[128];

        if (ent->vtable->build)
                ent->vtable->build(ent, &ent->block->DBD);

        if (GLLC_ENT_FLAG(ent, GLLC_ENT_SELECTED))
        {
                if (!ent->vtable->vertices)
                        return;

                ver_count = ent->vtable->vertices(ent, NULL);
                if (ver_count < 64)
                        ver = ver_stack;
                else
                        ver = malloc(sizeof(double) * 2 * ver_count);
                if (!ver)
                        return;

                ent->vtable->vertices(ent, ver);
                build_vertices(ent, ver, ver_count);

                if (ver_count >= 256)
                        free(ver);
        }
        else
        {
                for (i = 0; i < ent->DE_verices_count; i++)
                {
                        gllc_DE_destroy(ent->DE_vertices[i]);
                        ent->DE_vertices[i] = NULL;
                }
                ent->DE_verices_count = 0;
        }
}

int gllc_ent_bbox(struct gllc_block_entity *ent, double *x0, double *y0, double *x1, double *y1)
{
        return ent->vtable->bbox ? ent->vtable->bbox(ent, x0, y0, x1, y1) : 0;
}

int gllc_ent_picked(struct gllc_block_entity *ent, double x, double y)
{
        return ent->vtable->picked ? ent->vtable->picked(ent, x, y) : 0;
}