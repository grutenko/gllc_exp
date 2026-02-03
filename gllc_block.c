#include "gllc_block.h"
#include "gllc_block_entity.h"
#include "gllc_circle.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"
#include "gllc_point.h"
#include "gllc_polyline.h"
#include "gllc_rect.h"
#include "gllc_sparse_grid.h"

#include <assert.h>
#include <math.h>
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
        double bbox_x0, bbox_y0, bbox_x1, bbox_y1;

        struct gllc_block_entity *ent = block->ent_head;
        while (ent)
        {
                if (GLLC_ENT_FLAG(ent, GLLC_ENT_MODIFIED))
                {
                        gllc_ent_build(ent);

                        if (GLLC_ENT_FLAG(ent, GLLC_ENT_GEOMETRY_MODIFIED))
                        {
                                int ok = gllc_ent_bbox(ent, &bbox_x0, &bbox_y0, &bbox_x1, &bbox_y1);

                                if (!GLLC_ENT_FLAG(ent, GLLC_ENT_INITIAL))
                                {
                                        gllc_SG_remove(&block->ent_grid, ent);
                                }

                                if (ok)
                                {
                                        gllc_SG_push(&block->ent_grid, ent, bbox_x0, bbox_y0, bbox_x1, bbox_y1);
                                        GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_INITIAL);
                                }
                        }

                        GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_GEOMETRY_MODIFIED);
                        GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_MODIFIED);
                }
                ent = ent->next;
        }
}

void gllc_block_ent_deselect_all(struct gllc_block *block)
{
        assert(block);

        struct gllc_block_entity *ent = block->ent_head;
        while (ent)
        {
                if (GLLC_ENT_FLAG(ent, GLLC_ENT_SELECTED))
                {
                        GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_SELECTED);
                        GLLC_ENT_SET_FLAG(ent, GLLC_ENT_MODIFIED);
                }

                ent = ent->next;
        }
}

void gllc_block_ent_deselect(struct gllc_block *block, struct gllc_block_entity *ent)
{
        assert(block);
        assert(ent);
        assert(ent->block == block);

        GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_SELECTED);
        GLLC_ENT_SET_FLAG(ent, GLLC_ENT_MODIFIED);
}

void gllc_block_ent_select(struct gllc_block *block, struct gllc_block_entity *ent, int exlusive)
{
        assert(block);
        assert(ent);
        assert(ent->block == block);

        if (exlusive)
        {
                gllc_block_ent_deselect_all(block);
        }

        GLLC_ENT_SET_FLAG(ent, GLLC_ENT_SELECTED);
        GLLC_ENT_SET_FLAG(ent, GLLC_ENT_MODIFIED);
}

static inline void _swap(int *a, int *b)
{
        int t = *a;
        *a = *b;
        *b = t;
}

static inline void _swapf(double *a, double *b)
{
        double t = *a;
        *a = *b;
        *b = t;
}

void gllc_block_ent_select_by_bbox(struct gllc_block *block, double bbox_x0, double bbox_y0, double bbox_x1, double bbox_y1)
{
        double bbox1_x0, bbox1_y0, bbox1_x1, bbox1_y1;

        int cx0 = ((int)floor(bbox_x0)) >> GLLC_SG_CELL_SHIFT;
        int cy0 = ((int)floor(bbox_y0)) >> GLLC_SG_CELL_SHIFT;
        int cx1 = ((int)floor(bbox_x1)) >> GLLC_SG_CELL_SHIFT;
        int cy1 = ((int)floor(bbox_y1)) >> GLLC_SG_CELL_SHIFT;

        if (cx0 > cx1)
                _swap(&cx0, &cx1);
        if (cy0 > cy1)
                _swap(&cy0, &cy1);
        if (bbox_x0 > bbox_x1)
                _swapf(&bbox_x0, &bbox_x1);
        if (bbox_y0 > bbox_y1)
                _swapf(&bbox_y0, &bbox_y1);

        int x, y, i, ok;

        // Проходимся по всем ячейкам входящим в bbox и выбираем все элементы для строгой проверки
        for (x = cx0; x <= cx1; x++)
        {
                for (y = cy0; y <= cy1; y++)
                {
                        struct gllc_SG_cell *cell = gllc_SG_cell_at(&block->ent_grid, x, y);
                        if (!cell)
                                continue;

                        // В ячейке элементы находятся в соответствии со своим order
                        // Но при выделении могут быть области ниже в порядке отрисовки но выходят за пределы bbox такие тоже выделяем

                        if (cell->ent_size == 0)
                                continue;

                        for (i = 0; i < cell->ent_size; i++)
                        {
                                if (GLLC_ENT_FLAG(cell->ent[i], GLLC_ENT_LOCKED | GLLC_ENT_HIDDEN))
                                        continue;

                                if (!gllc_ent_bbox(cell->ent[i], &bbox1_x0, &bbox1_y0, &bbox1_x1, &bbox1_y1))
                                        continue;

                                if (bbox1_x0 > bbox1_x1)
                                        _swapf(&bbox1_x0, &bbox1_x1);
                                if (bbox1_y0 > bbox1_y1)
                                        _swapf(&bbox1_y0, &bbox1_y1);

                                if (!(bbox_x0 <= bbox1_x0 && bbox_y0 <= bbox1_y0 && bbox_x1 >= bbox1_x1 && bbox_y1 >= bbox1_y1))
                                        continue;

                                GLLC_ENT_SET_FLAG(cell->ent[i], GLLC_ENT_SELECTED);
                                GLLC_ENT_SET_FLAG(cell->ent[i], GLLC_ENT_MODIFIED);
                        }
                }
        }
}

struct gllc_block_entity *gllc_block_select_ent_by_point(struct gllc_block *block, double x, double y, int exlusive)
{
        struct gllc_block_entity *ent = gllc_block_pick_ent(block, x, y);
        if (ent)
        {
                gllc_block_ent_select(block, ent, exlusive);
        }
        return ent;
}

void gllc_block_destroy(struct gllc_block *block)
{
        struct gllc_block_entity *ent = block->ent_head;
        while (ent)
        {
                struct gllc_block_entity *next = ent->next;
                gllc_ent_destroy(ent);
                ent = next;
        }
        gllc_DBD_destroy(&block->DBD);
        gllc_object_cleanup(&block->__obj);
        gllc_SG_cleanup(&block->ent_grid);
        free(block);
}

struct gllc_block_entity *gllc_block_pick_ent(struct gllc_block *block, double x, double y)
{
        struct gllc_block_entity *ent = NULL;
        struct gllc_SG_cell *cell = gllc_SG_pick_cell(&block->ent_grid, x, y);

        if (!cell)
                return NULL;

        int i;
        for (i = 0; i < cell->ent_size; i++)
        {
                if (cell->ent[i]->vtable->picked(cell->ent[i], x, y))
                {
                        ent = cell->ent[i];
                        break;
                }
        }

        return ent;
}