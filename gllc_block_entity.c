#include "gllc_block_entity.h"
#include "gllc_block.h"
#include "gllc_layer.h"
#include "gllc_object.h"

#include <stdlib.h>

const struct gllc_prop_def g_block_entity_prop_def[] = {};

int gllc_block_entity_color(struct gllc_block_entity *ent) {
  if (ent->props.color >= 0)
    return ent->props.color;
  if (ent->layer && ent->layer->props.color >= 0)
    return ent->layer->props.color;
  if (ent->block->props.color >= 0)
    return ent->block->props.color;
  return 0;
}

int gllc_block_entity_fcolor(struct gllc_block_entity *ent) {
  if (ent->props.fcolor >= 0)
    return ent->props.fcolor;
  if (ent->layer && ent->layer->props.fcolor >= 0)
    return ent->layer->props.fcolor;
  if (ent->block->props.fcolor >= 0)
    return ent->block->props.fcolor;
  return 0;
}

void gllc_block_entity_destroy(struct gllc_block_entity *ent) {
  if (ent->destroy) {
    ent->destroy(ent);
  }
  free(ent);
}