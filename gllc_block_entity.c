#include "gllc_block_entity.h"
#include "gllc_block.h"
#include "gllc_layer.h"
#include "gllc_object.h"

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
        {
                GLLC_ENT_SET_FLAG(ent, GLLC_ENT_MODIFIED);
        }
}

void gllc_ent_destroy(struct gllc_block_entity *ent)
{
        if (ent->vtable->destroy)
        {
                ent->vtable->destroy(ent);
        }
        free(ent);
}

struct gllc_block_entity *gllc_ent_get_next(struct gllc_block_entity *ent)
{
        if (!ent)
        {
                return NULL;
        }
        return ent->next;
}