#include "gllc_text.h"
#include "gllc_block_entity.h"
#include "stb_truetype.h"

#include <stdlib.h>
#include <string.h>

static struct gllc_prop_def g_internal_props_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_block_entity_prop_def,
                                                    g_internal_props_def, 0};

static void _build(struct gllc_block_entity *ent, double scale,
                   struct gllc_DBD *DBD) {}

static void _destruct(struct gllc_block_entity *ent) {}

static int _bbox(struct gllc_block_entity *ent, double scale, double *bbox_x0,
                 double *bbox_y0, double *bbox_x1, double *bbox_y1) {
  return 0;
}

static int _picked(struct gllc_block_entity *ent, double scale, double x,
                   double y) {
  return 0;
}

static int _selected(struct gllc_block_entity *ent, double scale, double x0,
                     double y0, double x1, double y1) {
  return 0;
}

static int _vertices(struct gllc_block_entity *ent, double scale, double *ver) {
  return 0;
}

const static struct gllc_block_entity_vtable g_vtable = {.build = _build,
                                                         .destroy = _destruct,
                                                         .bbox = _bbox,
                                                         .picked = _picked,
                                                         .selected = _selected,
                                                         .vertices = _vertices,
                                                         .type = GLLC_ENT_TEXT};

struct gllc_text *gllc_text_create(struct gllc_block *block, const char *text,
                                   double x, double y, double width,
                                   double height, double angle) {
  struct gllc_text *ent = malloc(sizeof(struct gllc_text));
  if (ent) {
    memset(ent, 0, sizeof(struct gllc_text));

    GLLC_ENT_INIT(ent, g_props_def, block, &g_vtable);

    ent->x = x;
    ent->y = y;
    ent->width = width;
    ent->height = height;
    ent->angle = angle;

    char *local_text = malloc(strlen(text) + 1);
    if (!local_text) {
      free(ent);
      return 0;
    }
    ent->text = strcpy(local_text, text);
    ent->text_size = strlen(text);
  }
  return ent;
}