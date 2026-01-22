#include "gllc_block.h"
#include "gllc_block_entity.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"
#include "gllc_polyline.h"

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
    block->props.color = -1;
    block->props.fcolor = -1;
    gllc_draw_init(&block->draw_batch);
  }
  return block;
}

void gllc_block_remove_ent(struct gllc_block *block,
                           struct gllc_block_entity *ent) {
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
                                              int closed) {
  struct gllc_polyline *pline = gllc_polyline_create(block, closed);
  if (pline) {
    push_ent(block, (struct gllc_block_entity *)pline);
  }
  return pline;
}

void gllc_block_update(struct gllc_block *block) {
  struct gllc_block_entity *ent = block->ent_head;
  while (ent) {
    if (ent->modified)
      ent->build(ent, &block->draw_batch);
    ent = ent->next;
  }
  gllc_draw_build(&block->draw_batch);
}

void gllc_block_destroy(struct gllc_block *block) {
  struct gllc_block_entity *ent = block->ent_head;
  while (ent) {
    struct gllc_block_entity *next = ent->next;
    gllc_block_entity_destroy(ent);
    ent = next;
  }
  gllc_draw_cleanup(&block->draw_batch);
  gllc_object_cleanup(&block->__obj);
  free(block);
}