#include "gllc_block.h"
#include "gllc_block_entity.h"
#include "gllc_circle.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"
#include "gllc_point.h"
#include "gllc_polyline.h"
#include "gllc_rect.h"
#include "gllc_sparse_grid.h"
#include "gllc_vertex.h"
#include "include/gllc_block_entity.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static const struct gllc_prop_def g_prop_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_prop_def, 0};

struct gllc_block *gllc_block_create(struct gllc_drawing *drawing,
                                     const char *name, double dx, double dy) {
  struct gllc_block *block = malloc(sizeof(struct gllc_block));
  if (block) {
    memset(block, 0, sizeof(struct gllc_block));
    block->__obj.prop_def = g_props_def;
    block->drawing = drawing;
    strncpy(block->name, name, 63);
    block->dx = dx;
    block->dy = dy;
    block->props.color = 0;
    block->props.fcolor = 0;
    block->scale = 1.0f;
    gllc_DBD_init(&block->DBD);
    gllc_DBD_init(&block->I_DBD);
  }
  return block;
}

struct gllc_block_entity *gllc_block_get_first_ent(struct gllc_block *block) {
  return block->ent_head;
}

void gllc_block_remove_ent(struct gllc_block *block,
                           struct gllc_block_entity *ent) {
  assert(block == ent->block);

  if (ent->block != block)
    return;
  if (ent->prev)
    ent->prev->next = ent->next;
  else
    block->ent_head = ent->next;
  if (ent->next)
    ent->next->prev = ent->prev;
  else
    block->ent_tail = ent->prev;
  block->ent_count--;
  ent->block = NULL;
  block->DBD.modified = 1;
}

static void push_ent(struct gllc_block *block, struct gllc_block_entity *ent) {
  ent->block = block;
  ent->prev = block->ent_tail;
  ent->next = NULL;
  if (block->ent_tail)
    block->ent_tail->next = ent;
  else
    block->ent_head = ent;
  block->ent_tail = ent;
  block->ent_count++;
}

struct gllc_polyline *gllc_block_add_polyline(struct gllc_block *block,
                                              int closed, int filled) {
  struct gllc_polyline *pline = gllc_polyline_create(block, closed, filled);
  if (pline) {
    push_ent(block, (struct gllc_block_entity *)pline);
  }
  return pline;
}

struct gllc_rect *gllc_block_add_rect(struct gllc_block *block, double x,
                                      double y, double width, double height,
                                      double angle, int filled) {
  struct gllc_rect *rect =
      gllc_rect_create(block, x, y, width, height, angle, filled);
  if (rect) {
    push_ent(block, (struct gllc_block_entity *)rect);
  }
  return rect;
}

struct gllc_circle *gllc_block_add_circle(struct gllc_block *block, double x,
                                          double y, double radius, int filled) {
  struct gllc_circle *circle = gllc_circle_create(block, x, y, radius, filled);
  if (circle) {
    push_ent(block, (struct gllc_block_entity *)circle);
  }
  return circle;
}

struct gllc_point *gllc_block_add_point(struct gllc_block *block, double x,
                                        double y) {
  struct gllc_point *point = gllc_point_create(block, x, y);
  if (point) {
    push_ent(block, (struct gllc_block_entity *)point);
  }
  return point;
}

static struct gllc_vertex *add_vertex(struct gllc_block *block, double x,
                                      double y,
                                      struct gllc_block_entity *owner) {
  struct gllc_vertex *vertex = gllc_vertex_create(block, x, y, owner);
  struct gllc_block_entity *ent = (struct gllc_block_entity *)vertex;
  if (ent) {
    ent->block = block;
    ent->prev = block->I_ent_tail;
    ent->next = NULL;
    if (block->I_ent_tail)
      block->I_ent_tail->next = ent;
    else
      block->I_ent_head = ent;
    block->I_ent_tail = ent;
    block->I_ent_count++;
  }
  return vertex;
}

static void remove_vertex(struct gllc_block *block,
                          struct gllc_vertex *vertex) {
  struct gllc_block_entity *ent = (struct gllc_block_entity *)vertex;

  assert(block == ent->block);

  if (ent->prev)
    ent->prev->next = ent->next;
  else
    block->I_ent_head = ent->next;
  if (ent->next)
    ent->next->prev = ent->prev;
  else
    block->I_ent_tail = ent->prev;

  block->I_ent_count--;
  ent->block = NULL;
}

void gllc_block_update(struct gllc_block *block) {
  double bbox_x0, bbox_y0, bbox_x1, bbox_y1;
  struct gllc_block_entity *ent;

  for (ent = block->ent_head; ent; ent = ent->next) {
    if (!GLLC_ENT_FLAG(ent, GLLC_ENT_MODIFIED))
      continue;

    gllc_ent_build(ent, block->scale, &block->DBD);

    if (GLLC_ENT_FLAG(ent, GLLC_ENT_GEOMETRY_MODIFIED)) {
      int ok = gllc_ent_bbox(ent, block->scale, &bbox_x0, &bbox_y0, &bbox_x1,
                             &bbox_y1);

      if (!GLLC_ENT_FLAG(ent, GLLC_ENT_INITIAL)) {
        gllc_SG_remove(&block->ent_grid, ent);
      }

      if (ok) {
        gllc_SG_push(&block->ent_grid, ent, bbox_x0, bbox_y0, bbox_x1, bbox_y1);
        GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_INITIAL);
      }
    }

    GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_GEOMETRY_MODIFIED);
    GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_MODIFIED);
  }

  for (ent = block->I_ent_head; ent; ent = ent->next) {
    if (!GLLC_ENT_FLAG(ent, GLLC_ENT_MODIFIED))
      continue;

    gllc_ent_build(ent, block->scale, &block->I_DBD);

    if (GLLC_ENT_FLAG(ent, GLLC_ENT_GEOMETRY_MODIFIED)) {
      int ok = gllc_ent_bbox(ent, block->scale, &bbox_x0, &bbox_y0, &bbox_x1,
                             &bbox_y1);

      if (!GLLC_ENT_FLAG(ent, GLLC_ENT_INITIAL)) {
        gllc_SG_remove(&block->I_ent_grid, ent);
      }

      if (ok) {
        gllc_SG_push(&block->I_ent_grid, ent, bbox_x0, bbox_y0, bbox_x1,
                     bbox_y1);
        GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_INITIAL);
      }
    }

    GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_GEOMETRY_MODIFIED);
    GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_MODIFIED);
  }
}

void gllc_block_ent_deselect_all(struct gllc_block *block) {
  assert(block);

  struct gllc_block_entity *ent = block->ent_head;
  while (ent) {
    if (GLLC_ENT_FLAG(ent, GLLC_ENT_SELECTED)) {
      GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_SELECTED);
      GLLC_ENT_SET_FLAG(ent, GLLC_ENT_MODIFIED);
    }

    ent = ent->next;
  }

  gllc_SG_remove_all(&block->I_ent_grid);

  ent = block->I_ent_head;
  while (ent) {
    struct gllc_block_entity *next = ent->next;
    if (ent->vtable->type == GLLC_ENT_VERTEX) {
      remove_vertex(block, (struct gllc_vertex *)ent);
      gllc_ent_destroy(ent);
    }
    ent = next;
  }
}

void gllc_block_ent_deselect(struct gllc_block *block,
                             struct gllc_block_entity *ent) {
  assert(block);
  assert(ent);
  assert(ent->block == block);

  if (GLLC_ENT_FLAG(ent, GLLC_ENT_SELECTED)) {
    GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_SELECTED);
    GLLC_ENT_SET_FLAG(ent, GLLC_ENT_MODIFIED);

    struct gllc_block_entity *iEnt = block->I_ent_head;
    while (iEnt) {
      struct gllc_block_entity *next = iEnt->next;
      if (iEnt->vtable->type == GLLC_ENT_VERTEX) {
        struct gllc_vertex *vertex = (struct gllc_vertex *)iEnt;
        if (vertex->owner == ent) {
          remove_vertex(block, vertex);

          gllc_SG_remove(&block->I_ent_grid, iEnt);
          gllc_ent_destroy(iEnt);
        }
      }
      iEnt = next;
    }
  }
}

void gllc_block_ent_select(struct gllc_block *block,
                           struct gllc_block_entity *ent, int exlusive) {
  assert(block);
  assert(ent);
  assert(ent->block == block);

  double vertex_stack[128];
  double *vertex;
  int vertex_count;
  int i;

  if (exlusive) {
    gllc_block_ent_deselect_all(block);
  }

  if (!GLLC_ENT_FLAG(ent, GLLC_ENT_SELECTED)) {
    GLLC_ENT_SET_FLAG(ent, GLLC_ENT_SELECTED);
    GLLC_ENT_SET_FLAG(ent, GLLC_ENT_MODIFIED);

    vertex_count = ent->vtable->vertices(ent, block->scale, NULL);
    if (vertex_count < 64)
      vertex = vertex_stack;
    else
      vertex = malloc(sizeof(double) * 2 * vertex_count);

    if (!vertex)
      return;

    ent->vtable->vertices(ent, block->scale, vertex);

    for (i = 0; i < vertex_count; i++) {
      add_vertex(block, vertex[i * 2], vertex[i * 2 + 1], ent);
    }
  }
}

static inline void _swap(int *a, int *b) {
  int t = *a;
  *a = *b;
  *b = t;
}

static inline void _swapf(double *a, double *b) {
  double t = *a;
  *a = *b;
  *b = t;
}

void gllc_block_ent_select_by_bbox(struct gllc_block *block, double bbox_x0,
                                   double bbox_y0, double bbox_x1,
                                   double bbox_y1) {
  gllc_block_ent_deselect_all(block);

  double bbox1_x0, bbox1_y0, bbox1_x1, bbox1_y1;

  int cx0 = ((int)floor(bbox_x0)) >> GLLC_SG_CELL_SHIFT;
  int cy0 = ((int)floor(bbox_y0)) >> GLLC_SG_CELL_SHIFT;
  int cx1 = ((int)floor(bbox_x1)) >> GLLC_SG_CELL_SHIFT;
  int cy1 = ((int)floor(bbox_y1)) >> GLLC_SG_CELL_SHIFT;
  int x, y, i, ok;

  if (cx0 > cx1)
    _swap(&cx0, &cx1);
  if (cy0 > cy1)
    _swap(&cy0, &cy1);
  if (bbox_x0 > bbox_x1)
    _swapf(&bbox_x0, &bbox_x1);
  if (bbox_y0 > bbox_y1)
    _swapf(&bbox_y0, &bbox_y1);

  // Проходимся по всем ячейкам входящим в bbox и выбираем все элементы для
  // строгой проверки
  for (x = cx0; x <= cx1; x++) {
    for (y = cy0; y <= cy1; y++) {
      struct gllc_SG_cell *cell = gllc_SG_cell_at(&block->ent_grid, x, y);
      if (!cell)
        continue;

      if (cell->ent_size == 0)
        continue;

      for (i = 0; i < cell->ent_size; i++) {
        // В ячейке элементы находятся в соответствии со своим order
        // Но при выделении могут быть области ниже в порядке отрисовки но
        // выходят за пределы bbox такие тоже выделяем Вобще это работает
        // немного неправильно: Мы выделяем все элемены в BBOX независимо от
        // уровня. возможно стоит искать выходы примитива Но это довольно сложно
        // и долго, так что не факт что нужно.

        if (GLLC_ENT_FLAG(cell->ent[i], GLLC_ENT_LOCKED | GLLC_ENT_HIDDEN))
          continue;

        // Bbox Учитывает scale_factor. Объекты в экранных координатах правильно
        // рассчитывают свой Bbox исходя из текущего scale_factor
        if (!gllc_ent_bbox(cell->ent[i], block->scale, &bbox1_x0, &bbox1_y0,
                           &bbox1_x1, &bbox1_y1))
          continue;

        if (bbox1_x0 > bbox1_x1)
          _swapf(&bbox1_x0, &bbox1_x1);
        if (bbox1_y0 > bbox1_y1)
          _swapf(&bbox1_y0, &bbox1_y1);

        // Для select by bbox достаточно просто проверить вхождение в Bbox
        // обьекта.
        if (!(bbox_x0 <= bbox1_x0 && bbox_y0 <= bbox1_y0 &&
              bbox_x1 >= bbox1_x1 && bbox_y1 >= bbox1_y1))
          continue;

        gllc_block_ent_select(block, cell->ent[i], 0);
      }
    }
  }
}

struct gllc_vertex *gllc_block_pick_vertex(struct gllc_block *block, double x,
                                           double y) {
  struct gllc_block_entity *ent = NULL;
  // Ищем подходящую ячейку в Sparce Grid
  struct gllc_SG_cell *cell = gllc_SG_pick_cell(&block->I_ent_grid, x, y);

  if (!cell)
    return NULL;

  int i;
  for (i = 0; i < cell->ent_size; i++) {
    // возможно дальше тут будут не только gllc_vertex. Но пока только они

    if (cell->ent[i]->vtable->type != GLLC_ENT_VERTEX)
      continue;

    // Перебираем все элементы пока один из них не вернут true на эту точку.
    // cell->ent[i]->vtable->picked() пишется под каждый примитив чтобы точно
    // определить вхождение точки в примитив. cell->ent - данные в порядке
    // order, поэтому первый попавшийся - выше визуально.

    if (cell->ent[i]->vtable->picked(cell->ent[i], block->scale, x, y)) {
      ent = cell->ent[i];
      break;
    }
  }

  return (struct gllc_vertex *)ent;
}

struct gllc_block_entity *
gllc_block_select_ent_by_point(struct gllc_block *block, double x, double y,
                               int exlusive) {
  struct gllc_block_entity *ent;
  ent = (struct gllc_block_entity *)gllc_block_pick_vertex(block, x, y);
  if (ent) {
    if (!GLLC_ENT_FLAG(ent, GLLC_ENT_SELECTED)) {
      struct gllc_block_entity *I_ent = block->I_ent_head;
      for (I_ent = block->I_ent_head; I_ent; I_ent = I_ent->next) {
        if (I_ent->vtable->type == GLLC_ENT_VERTEX &&
            GLLC_ENT_FLAG(I_ent, GLLC_ENT_SELECTED)) {
          GLLC_ENT_UNSET_FLAG(I_ent, GLLC_ENT_SELECTED);
          GLLC_ENT_SET_FLAG(I_ent, GLLC_ENT_MODIFIED);
        }
      }

      GLLC_ENT_SET_FLAG(ent, GLLC_ENT_SELECTED);
      GLLC_ENT_SET_FLAG(ent, GLLC_ENT_MODIFIED);
    }
    return ent;
  }
  ent = gllc_block_pick_ent(block, x, y);
  if (ent) {
    gllc_block_ent_select(block, ent, exlusive);
  }
  return ent;
}

void gllc_block_destroy(struct gllc_block *block) {
  struct gllc_block_entity *ent = block->ent_head;
  while (ent) {
    struct gllc_block_entity *next = ent->next;
    gllc_ent_destroy(ent);
    ent = next;
  }
  gllc_DBD_destroy(&block->DBD);
  gllc_object_cleanup(&block->__obj);
  gllc_SG_cleanup(&block->ent_grid);
  free(block);
}

struct gllc_block_entity *gllc_block_pick_ent(struct gllc_block *block,
                                              double x, double y) {
  struct gllc_block_entity *ent = NULL;
  struct gllc_SG_cell *cell = gllc_SG_pick_cell(&block->ent_grid, x, y);

  if (!cell)
    return NULL;

  int i;
  for (i = 0; i < cell->ent_size; i++) {
    if (cell->ent[i]->vtable->picked(cell->ent[i], block->scale, x, y)) {
      ent = cell->ent[i];
      break;
    }
  }

  return ent;
}

void gllc_block_on_scale(struct gllc_block *block, double new_scale) {
  block->scale = new_scale;
}