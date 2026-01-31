#include "gllc_block.h"
#include "gllc_block_entity.h"
#include "gllc_circle.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"
#include "gllc_polyline.h"
#include "gllc_rect.h"
#include "include/gllc_point.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static const struct gllc_prop_def g_prop_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_prop_def, 0};

struct gllc_block *gllc_block_create(struct gllc_drawing *drawing,
                                     const char *name, double dx, double dy)
{
        struct gllc_block *block = malloc(sizeof(struct gllc_block));
        if (block)
        {
                memset(block, 0, sizeof(struct gllc_block));
                block->__obj.prop_def = g_props_def;
                block->drawing = drawing;
                strncpy(block->name, name, 63);
                block->dx = dx;
                block->dy = dy;
                block->props.color = 0;
                block->props.fcolor = 0;
                gllc_DBD_init(&block->DBD);
        }
        return block;
}

struct gllc_block_entity *gllc_block_get_first_ent(struct gllc_block *block)
{
        return block->ent_head;
}

void gllc_block_remove_ent(struct gllc_block *block,
                           struct gllc_block_entity *ent)
{
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

static void push_ent(struct gllc_block *block, struct gllc_block_entity *ent)
{
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

struct gllc_polyline *gllc_block_add_polyline(struct gllc_block *block, int closed, int filled)
{
        struct gllc_polyline *pline = gllc_polyline_create(block, closed, filled);
        if (pline)
        {
                push_ent(block, (struct gllc_block_entity *)pline);
        }
        return pline;
}

struct gllc_rect *gllc_block_add_rect(struct gllc_block *block, double x, double y, double width, double height, double angle, int filled)
{
        struct gllc_rect *rect = gllc_rect_create(block, x, y, width, height, angle, filled);
        if (rect)
        {
                push_ent(block, (struct gllc_block_entity *)rect);
        }
        return rect;
}

struct gllc_circle *gllc_block_add_circle(struct gllc_block *block, double x, double y, double radius, int filled)
{
        struct gllc_circle *circle = gllc_circle_create(block, x, y, radius, filled);
        if (circle)
        {
                push_ent(block, (struct gllc_block_entity *)circle);
        }
        return circle;
}

struct gllc_point *gllc_block_add_point(struct gllc_block *block, double x, double y)
{
        struct gllc_point *point = gllc_point_create(block, x, y);
        if (point)
        {
                push_ent(block, (struct gllc_block_entity *)point);
        }
        return point;
}

void gllc_block_update(struct gllc_block *block)
{
        struct gllc_block_entity *ent = block->ent_head;
        while (ent)
        {
                if (ent->modified)
                {
                        ent->build(ent, &block->DBD);
                }
                ent = ent->next;
        }
}

void gllc_block_destroy(struct gllc_block *block)
{
        struct gllc_block_entity *ent = block->ent_head;
        while (ent)
        {
                struct gllc_block_entity *next = ent->next;
                gllc_block_entity_destroy(ent);
                ent = next;
        }
        gllc_DBD_destroy(&block->DBD);
        gllc_object_cleanup(&block->__obj);
        free(block);
}