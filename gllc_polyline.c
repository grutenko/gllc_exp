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
  if (pline->draw_bound)
    gllc_draw_ent_remove(pline->draw_bound->buffer, pline->draw_bound);
  if (pline->draw_fill)
    gllc_draw_ent_remove(pline->draw_fill->buffer, pline->draw_fill);
  if (pline->draw_bound)
    gllc_draw_ent_destroy(pline->draw_bound);
  if (pline->draw_fill)
    gllc_draw_ent_destroy(pline->draw_fill);
  free(pline->ver);
}

static GLfloat *G_vertices = NULL;
static GLuint *G_indices = NULL;
static size_t G_vcap = 0;
static size_t G_icap = 0;

static void build(struct gllc_block_entity *ent, struct gllc_draw_batch *draw) {
  struct gllc_polyline *pline = (struct gllc_polyline *)ent;
  size_t vert_count = pline->ver_size / 2;
  size_t vert_size = sizeof(GLfloat) * pline->ver_size;
  size_t ind_size = sizeof(GLuint) * vert_count;
  if (G_vcap < vert_size) {
    GLfloat *p = malloc(vert_size);
    if (!p)
      return;
    free(G_vertices);
    G_vertices = p;
    G_vcap = vert_size;
  }
  for (size_t i = 0; i < pline->ver_size; i++) {
    G_vertices[i] = (float)pline->ver[i];
  }
  if (G_icap < ind_size) {
    GLuint *p = malloc(ind_size);
    if (!p)
      return;
    free(G_indices);
    G_indices = p;
    G_icap = ind_size;
  }
  for (size_t i = 0; i < vert_count; i++) {
    G_indices[i] = i;
  }
  struct gllc_draw_ent_config ent_cfg;
  ent_cfg.v = G_vertices;
  ent_cfg.v_size = pline->ver_size;
  ent_cfg.i = G_indices;
  ent_cfg.i_size = vert_count;
  int color = gllc_block_entity_color(ent);
  ent_cfg.color[0] = (float)(color >> 16) / 255;
  ent_cfg.color[1] = (float)(color >> 8) / 255;
  ent_cfg.color[2] = (float)color / 255;
  ent_cfg.color[3] = 1.0f;
  gllc_draw_ent_update(pline->draw_bound, &ent_cfg);
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
      ent->draw_bound =
          gllc_draw_buffer_push_ent(&block->draw_batch.gl_line_loop, &ent_config);
      ent->draw_fill =
          gllc_draw_buffer_push_ent(&block->draw_batch.gl_triangles, &ent_config);
      assert(ent->draw_bound);
      assert(ent->draw_fill);
    } else {
      ent->draw_bound =
          gllc_draw_buffer_push_ent(&block->draw_batch.gl_line_strip, &ent_config);
      assert(ent->draw_bound);
    }
    ent->closed = closed;
    ent->__ent.props.color = -1;
    ent->__ent.props.fcolor = -1;
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