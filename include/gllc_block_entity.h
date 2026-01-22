#ifndef gllc_block_entity_h
#define gllc_block_entity_h

#include "gllc_object.h"

struct gllc_draw_ent;
struct gllc_draw_batch;
struct gllc_block;

struct gllc_block_entity_props {
  int color;
  int fcolor;
};

struct gllc_block_entity;

typedef void (*gllc_block_ent_build_cb)(struct gllc_block_entity *,
                                        struct gllc_draw_batch *);
typedef void (*gllc_block_ent_destroy_cb)(struct gllc_block_entity *);

struct gllc_block_entity {
  struct gllc_object __obj;
  struct gllc_block_entity_props props;
  struct gllc_block *block;
  struct gllc_layer *layer;
  gllc_block_ent_build_cb build;
  gllc_block_ent_destroy_cb destroy;
  struct gllc_block_entity *next;
  struct gllc_block_entity *prev;
  int flags;
  int modified;
};

void gllc_block_entity_destroy(struct gllc_block_entity *ent);
int gllc_block_entity_color(struct gllc_block_entity *ent);
int gllc_block_entity_fcolor(struct gllc_block_entity *ent);

extern const struct gllc_prop_def g_block_entity_prop_def[];

#endif