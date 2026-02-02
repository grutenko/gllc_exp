#ifndef gllc_block_entity_h
#define gllc_block_entity_h

#include "gllc_draw_buffer.h"
#include "gllc_object.h"

enum
{
        GLLC_ENT_CIRCLE = 1,
        GLLC_ENT_POINT = 2,
        GLLC_ENT_RECT = 3,
        GLLC_ENT_POLYLINE = 4
};

#define GLLC_ENT_CLOSED 0x1
#define GLLC_ENT_FILLED 0x2
#define GLLC_ENT_MODIFIED 0x4
#define GLLC_ENT_LOCKED 0x8
#define GLLC_ENT_HIDDEN 0x10
#define GLLC_ENT_INITIAL 0x20
#define GLLC_ENT_SELECTED 0x40

struct gllc_block;

struct gllc_block_entity;

typedef void (*gllc_block_ent_build_cb)(struct gllc_block_entity *, struct gllc_DBD *);
typedef void (*gllc_block_ent_destroy_cb)(struct gllc_block_entity *);
typedef int (*gllc_block_ent_bbox_cb)(struct gllc_block_entity *, double *, double *, double *, double *);
typedef int (*gllc_block_ent_picked_cb)(struct gllc_block_entity *, double, double);
typedef int (*gllc_block_ent_selected_cb)(struct gllc_block_entity *, double, double, double, double);

struct gllc_block_entity_vtable
{
        gllc_block_ent_build_cb build;
        gllc_block_ent_destroy_cb destroy;
        gllc_block_ent_bbox_cb bbox;
        gllc_block_ent_picked_cb picked;
        gllc_block_ent_selected_cb selected;
        int type;
};

struct gllc_block_entity_props
{
        int color;
        int fcolor;
};

struct gllc_block_entity
{
        struct gllc_object __obj;
        const struct gllc_block_entity_vtable *vtable;
        struct gllc_block_entity_props props;
        struct gllc_block *block;
        struct gllc_layer *layer;
        struct gllc_block_entity *next;
        struct gllc_block_entity *prev;
        int order;
        unsigned int flags;
};

#define GLLC_ENT_INIT(ent_, props_, block_, vtable_)                             \
        do                                                                       \
        {                                                                        \
                ((struct gllc_block_entity *)(ent_))->__obj.prop_def = (props_); \
                ((struct gllc_block_entity *)(ent_))->block = (block_);          \
                ((struct gllc_block_entity *)(ent_))->vtable = (vtable_);        \
                ((struct gllc_block_entity *)(ent_))->props.color = -1;          \
                ((struct gllc_block_entity *)(ent_))->props.fcolor = -1;         \
                GLLC_ENT_SET_FLAG(ent_, GLLC_ENT_MODIFIED);                      \
                GLLC_ENT_SET_FLAG(ent_, GLLC_ENT_INITIAL);                       \
        } while (0)

#define GLLC_ENT_SET_FLAG(ent, fl) (((struct gllc_block_entity *)ent)->flags |= (fl))
#define GLLC_ENT_UNSET_FLAG(ent, fl) (((struct gllc_block_entity *)ent)->flags &= ~(fl))
#define GLLC_ENT_FLAG(ent, fl) (((struct gllc_block_entity *)ent)->flags & (fl))

void gllc_ent_color_4f(int color, float *out_);

void gllc_ent_destroy(struct gllc_block_entity *ent);

int gllc_ent_color(struct gllc_block_entity *ent);

void gllc_ent_set_color(struct gllc_block_entity *ent, int color);

int gllc_ent_fcolor(struct gllc_block_entity *ent);

void gllc_ent_set_fcolor(struct gllc_block_entity *ent, int color);

void gllc_ent_set_layer(struct gllc_block_entity *ent, struct gllc_layer *layer);

struct gllc_block_entity *gllc_ent_get_next(struct gllc_block_entity *ent);

extern const struct gllc_prop_def g_block_entity_prop_def[];

#endif