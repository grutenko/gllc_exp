#include "gllc_vertex.h"
#include "gllc_block_entity.h"
#include "gllc_draw_buffer.h"

#include <stdlib.h>
#include <string.h>

static const struct gllc_prop_def g_internal_props_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_block_entity_prop_def,
                                                    g_internal_props_def, 0};

static void _build(struct gllc_block_entity *ent, double scale,
                   struct gllc_DBD *DBD) {
  struct gllc_vertex *vertex = (struct gllc_vertex *)ent;

  if (!vertex->DE_fill) {
    vertex->DE_fill = gllc_DE_create(DBD, GL_TRIANGLES);
    if (!vertex->DE_fill)
      return;
  }

  if (GLLC_ENT_FLAG(ent, GLLC_ENT_SELECTED)) {
    if (!vertex->DE_sel) {
      vertex->DE_sel = gllc_DE_create(DBD, GL_LINE_LOOP);
      if (!vertex->DE_sel)
        return;
    }

    GLfloat V[] = {
        vertex->x - 4.5f, vertex->y - 4.5f, vertex->x + 4.5f, vertex->y - 4.5f,
        vertex->x + 4.5f, vertex->y + 4.5f, vertex->x - 4.5f, vertex->y + 4.5f,
    };
    GLuint I[] = {0, 1, 2, 3};
    GLuint flags = GLLC_POINT_SCALE_INVARIANT;
    GLfloat color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat center_point[] = {(GLfloat)vertex->x, (GLfloat)vertex->y};

    struct gllc_DE_config conf = {.V = V,
                                  .I = I,
                                  .V_count = 4,
                                  .I_count = 4,
                                  .center_point = center_point,
                                  .color = color,
                                  .flags = &flags};

    gllc_DE_update(vertex->DE_sel, &conf);
  } else {
    if (vertex->DE_sel) {
      gllc_DE_destroy(vertex->DE_sel);
      vertex->DE_sel = NULL;
    }
  }

  GLfloat V[] = {
      vertex->x - 3.0f, vertex->y - 3.0f, vertex->x + 3.0f, vertex->y - 3.0f,
      vertex->x + 3.0f, vertex->y + 3.0f, vertex->x - 3.0f, vertex->y + 3.0f,
  };
  GLuint I[] = {0, 1, 2, 0, 2, 3};
  GLuint flags = GLLC_POINT_SCALE_INVARIANT;
  GLfloat color[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat center_point[] = {(GLfloat)vertex->x, (GLfloat)vertex->y};

  struct gllc_DE_config conf = {.V = V,
                                .I = I,
                                .V_count = 4,
                                .I_count = 6,
                                .center_point = center_point,
                                .color = color,
                                .flags = &flags};

  gllc_DE_update(vertex->DE_fill, &conf);
}

static void _destroy(struct gllc_block_entity *ent) {
  if (((struct gllc_vertex *)ent)->DE_fill)
    gllc_DE_destroy(((struct gllc_vertex *)ent)->DE_fill);
  if (((struct gllc_vertex *)ent)->DE_sel)
    gllc_DE_destroy(((struct gllc_vertex *)ent)->DE_sel);
}

static int _bbox(struct gllc_block_entity *ent, double scale, double *bbox_x0,
                 double *bbox_y0, double *bbox_x1, double *bbox_y1) {
  struct gllc_vertex *vertex = (struct gllc_vertex *)ent;

  *bbox_x0 = vertex->x - (3.0f * scale);
  *bbox_y0 = vertex->y - (3.0f * scale);
  *bbox_x1 = vertex->x + (3.0f * scale);
  *bbox_y1 = vertex->y + (3.0f * scale);

  return 1;
}

static int _picked(struct gllc_block_entity *ent, double scale, double x,
                   double y) {
  struct gllc_vertex *v = (struct gllc_vertex *)ent;

  double x0 = v->x - 3.0f * scale;
  double y0 = v->y - 3.0f * scale;
  double x1 = v->x + 3.0f * scale;
  double y1 = v->y + 3.0f * scale;

  return x0 <= x && y0 <= y && x1 >= x && y1 >= y;
}

static int _selected(struct gllc_block_entity *ent, double scale, double x0,
                     double y0, double x1, double y1) {
  // not used
  return 0;
}

static int _vertices(struct gllc_block_entity *ent, double scale, double *ver) {
  // not used
  return 0;
}

static const struct gllc_block_entity_vtable g_vtable = {.build = _build,
                                                         .bbox = _bbox,
                                                         .destroy = _destroy,
                                                         .picked = _picked,
                                                         .selected = _selected,
                                                         .vertices = _vertices,
                                                         .type =
                                                             GLLC_ENT_VERTEX};

struct gllc_vertex *gllc_vertex_create(struct gllc_block *block, double x,
                                       double y,
                                       struct gllc_block_entity *owner) {
  struct gllc_vertex *vertex = malloc(sizeof(struct gllc_vertex));
  if (vertex) {
    memset(vertex, 0, sizeof(struct gllc_vertex));

    GLLC_ENT_INIT(vertex, g_props_def, block, &g_vtable);

    vertex->x = x;
    vertex->y = y;
    vertex->owner = owner;
  }
  return vertex;
}