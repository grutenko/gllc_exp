#ifndef gllc_layer_h
#define gllc_layer_h

#include "gllc_block_entity.h"
#include "gllc_object.h"

struct gllc_block;

struct gllc_layer {
  struct gllc_object __obj;
  struct gllc_block_entity_props props;
  struct gllc_block *block;
  struct gllc_layer *next;
  struct gllc_layer *prev;
};

struct gllc_layer *gllc_layer_create(struct gllc_block *block);
void gllc_layer_destroy(struct gllc_layer *layer);

#endif