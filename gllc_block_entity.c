#include "gllc_block_entity.h"
#include "gllc_block.h"
#include "gllc_object.h"

#include <stdlib.h>

const struct gllc_prop_def g_block_entity_prop_def[] = {};

void gllc_block_entity_destroy(struct gllc_block_entity *ent) {
  if (ent->destroy) {
    ent->destroy(ent);
  }
  free(ent);
}