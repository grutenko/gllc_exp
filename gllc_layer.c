#include "gllc_layer.h"
#include "gllc_object.h"
#include <stdlib.h>
#include <string.h>

static const struct gllc_prop_def g_prop_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_prop_def, 0};

struct gllc_layer *gllc_layer_create(struct gllc_block *block) {
  struct gllc_layer *layer = malloc(sizeof(struct gllc_layer));
  if (layer) {
    memset(layer, 0, sizeof(struct gllc_layer));
    layer->__obj.prop_def = g_props_def;
    layer->block = block;
    layer->props.color = -1;
    layer->props.fcolor = -1;
  }
  return layer;
}

void gllc_layer_destroy(struct gllc_layer *layer) {
  gllc_object_cleanup(&layer->__obj);
  free(layer);
}