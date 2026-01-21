#include "gllc_polyline.h"
#include "gllc_block.h"
#include "gllc_block_entity.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct gllc_prop_def g_internal_props_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_block_entity_prop_def,
                                                    g_internal_props_def, 0};

static void destruct(struct gllc_block_entity *ent) {
  struct gllc_polyline *pline = (struct gllc_polyline *)ent;
  if (pline->bound)
    gllc_draw_ent_remove(pline->bound->buffer, pline->bound);
  if (pline->fill)
    gllc_draw_ent_remove(pline->fill->buffer, pline->fill);
  if (pline->bound)
    gllc_draw_ent_destroy(pline->bound);
  if (pline->fill)
    gllc_draw_ent_destroy(pline->fill);
  free(pline->ver);
}

static void build(struct gllc_block_entity *ent, struct gllc_draw *draw) {
  struct gllc_polyline *pline = (struct gllc_polyline *)ent;
  size_t vert_count = pline->ver_size / 2;
  GLfloat *v_float = malloc(sizeof(GLfloat) * pline->ver_size);
  if (!v_float)
    return;
  GLuint *indices = malloc(sizeof(GLuint) * (pline->ver_size / 2));
  if (!indices)
    return;
  for (size_t i = 0; i < pline->ver_size / 2; i++)
    indices[i] = i;
  struct gllc_draw_ent_config ent_cfg;
  ent_cfg.v = v_float;
  ent_cfg.v_size = pline->ver_size;
  ent_cfg.i = indices;
  ent_cfg.i_size = pline->ver_size / 2;
  printf("pline->ver_size = %zu\n", pline->ver_size);
  ent_cfg.color[0] = 1.0f; // например красный
  ent_cfg.color[1] = 0.0f;
  ent_cfg.color[2] = 0.0f;
  ent_cfg.color[3] = 1.0f;
  int rc = gllc_draw_ent_update(pline->bound, &ent_cfg);
  if(rc) {
    printf("Polyline succesfully updateed.\n");
  }
  free(v_float);
  free(indices);
  pline->__ent.modified = 0;
}

struct gllc_polyline *gllc_polyline_create(struct gllc_block *block,
                                           int closed) {
  struct gllc_polyline *ent = malloc(sizeof(struct gllc_polyline));
  if (ent) {
    struct gllc_draw_ent_config ent_config = {0};
    memset(ent, 0, sizeof(struct gllc_polyline));
    ent->__ent.__obj.prop_def = g_props_def;
    ent->__ent.destroy = destruct;
    ent->__ent.build = build;
    ent->__ent.block = block;
    if (closed) {
      ent->bound =
          gllc_draw_buffer_push_ent(&block->draw.gl_line_loop, &ent_config);
      ent->fill =
          gllc_draw_buffer_push_ent(&block->draw.gl_triangles, &ent_config);
      assert(ent->bound);
      assert(ent->fill);
    } else {
      ent->bound =
          gllc_draw_buffer_push_ent(&block->draw.gl_line_strip, &ent_config);
      assert(ent->bound);
    }
    ent->closed = closed;
    ent->__ent.modified = 1;
  }
  return ent;
}
static int push_ver(struct gllc_polyline *pline, double x, double y) {
  if (pline->ver_size + 2 > pline->ver_cap) {
    size_t new_cap = pline->ver_cap ? pline->ver_cap * 2 : 8;
    double *new_ver = (double *)realloc(pline->ver, sizeof(double) * new_cap);
    if (!new_ver)
      return 0;
    pline->ver = new_ver;
    pline->ver_cap = new_cap;
  }
  pline->ver[pline->ver_size++] = x;
  pline->ver[pline->ver_size++] = y;
  return 1;
}

void gllc_polyline_add_ver(struct gllc_polyline *pline, double x, double y) {
  push_ver(pline, x, y);
  pline->__ent.modified = 1;
}