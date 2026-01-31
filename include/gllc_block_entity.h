#ifndef gllc_block_entity_h
#define gllc_block_entity_h

#include "gllc_draw_buffer.h"
#include "gllc_object.h"

#define GLLC_ENT_CLOSED 0x1
#define GLLC_ENT_FILLED 0x2

struct gllc_draw_ent;
struct gllc_draw_batch;
struct gllc_block;

struct gllc_block_entity_props
{
        int color;
        int fcolor;
};

struct gllc_block_entity;

typedef void (*gllc_block_ent_build_cb)(struct gllc_block_entity *, struct gllc_DBD *);
typedef void (*gllc_block_ent_destroy_cb)(struct gllc_block_entity *);

struct gllc_block_entity
{
        struct gllc_object __obj;
        struct gllc_block_entity_props props;
        struct gllc_block *block;
        struct gllc_layer *layer;
        gllc_block_ent_build_cb build;
        gllc_block_ent_destroy_cb destroy;
        struct gllc_block_entity *next;
        struct gllc_block_entity *prev;
        double BBox_x0;
        double BBox_x1;
        double BBox_y0;
        double Bbox_y1;
        unsigned int flags;
        int modified;
};

#define GLLC_ENT_SET_FLAG(ent, fl) (((struct gllc_block_entity *)ent)->flags |= (fl))
#define GLLC_ENT_UNSET_FLAG(ent, fl) (((struct gllc_block_entity *)ent)->flags &= ~(fl))
#define GLLC_ENT_FLAG(ent, fl) (((struct gllc_block_entity *)ent)->flags & (fl))

void gllc_block_entity_destroy(struct gllc_block_entity *ent);

int gllc_block_entity_color(struct gllc_block_entity *ent);

void gllc_block_entity_set_color(struct gllc_block_entity *ent, int color);

int gllc_block_entity_fcolor(struct gllc_block_entity *ent);

void gllc_block_entity_set_fcolor(struct gllc_block_entity *ent, int color);

void gllc_block_entity_set_layer(struct gllc_block_entity *ent, struct gllc_layer *layer);

struct gllc_block_entity *gllc_block_entity_get_next(struct gllc_block_entity *ent);

extern const struct gllc_prop_def g_block_entity_prop_def[];

#endif