#include "gllc_block.h"
#include "gllc_block_entity.h"
#include "gllc_circle.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"
#include "gllc_point.h"
#include "gllc_polyline.h"
#include "gllc_rect.h"
#include "gllc_sparse_grid.h"
#include "include/gllc_vertex.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
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
                block->scale = 1.0f;
                gllc_DBD_init(&block->DBD);
        }
        return block;
}

struct gllc_block_entity *gllc_block_get_first_ent(struct gllc_block *block)
{
        return block->ent_head;
}

static void remove_ent(struct gllc_block *block, struct gllc_block_entity *ent, int interactive)
{
        struct gllc_block_entity **head;
        struct gllc_block_entity **tail;
        size_t *cnt;

        if (interactive)
        {
                head = &block->I_ent_head;
                tail = &block->I_ent_tail;
                cnt = &block->I_ent_count;
        }
        else
        {
                head = &block->ent_head;
                tail = &block->ent_tail;
                cnt = &block->ent_count;
        }

        if (ent->block != block)
                return;
        if (ent->prev)
                ent->prev->next = ent->next;
        else
                *head = ent->next;
        if (ent->next)
                ent->next->prev = ent->prev;
        else
                *tail = ent->prev;
        (*cnt)--;
        ent->block = NULL;
}

void gllc_block_remove_ent(struct gllc_block *block,
                           struct gllc_block_entity *ent)
{
        assert(block == ent->block);

        gllc_SG_remove(&block->ent_grid, ent);
        remove_ent(block, ent, 0);
        block->DBD.modified = 1;
}

static void push_ent(struct gllc_block *block, struct gllc_block_entity *ent, int interactive)
{
        struct gllc_block_entity **head;
        struct gllc_block_entity **tail;
        size_t *cnt;

        if (interactive)
        {
                head = &block->I_ent_head;
                tail = &block->I_ent_tail;
                cnt = &block->I_ent_count;
        }
        else
        {
                head = &block->ent_head;
                tail = &block->ent_tail;
                cnt = &block->ent_count;
        }

        ent->block = block;
        ent->prev = *tail;
        ent->next = NULL;
        if (*tail)
                (*tail)->next = ent;
        else
                (*head) = ent;
        (*tail) = ent;
        (*cnt)++;
}

struct gllc_polyline *gllc_block_add_polyline(struct gllc_block *block, int closed, int filled)
{
        struct gllc_polyline *pline = gllc_polyline_create(block, closed, filled);
        if (pline)
        {
                push_ent(block, (struct gllc_block_entity *)pline, 0);
        }
        return pline;
}

struct gllc_rect *gllc_block_add_rect(struct gllc_block *block, double x, double y, double width, double height, double angle, int filled)
{
        struct gllc_rect *rect = gllc_rect_create(block, x, y, width, height, angle, filled);
        if (rect)
        {
                push_ent(block, (struct gllc_block_entity *)rect, 0);
        }
        return rect;
}

struct gllc_circle *gllc_block_add_circle(struct gllc_block *block, double x, double y, double radius, int filled)
{
        struct gllc_circle *circle = gllc_circle_create(block, x, y, radius, filled);
        if (circle)
        {
                push_ent(block, (struct gllc_block_entity *)circle, 0);
        }
        return circle;
}

struct gllc_point *gllc_block_add_point(struct gllc_block *block, double x, double y)
{
        struct gllc_point *point = gllc_point_create(block, x, y);
        if (point)
        {
                push_ent(block, (struct gllc_block_entity *)point, 0);
        }
        return point;
}

void gllc_block_update(struct gllc_block *block)
{
        int i;
        double ver_stack[128];
        double bbox_x0, bbox_y0, bbox_x1, bbox_y1;
        struct gllc_block_entity *ent;

        for (ent = block->ent_head; ent; ent = ent->next)
        {
                if (!GLLC_ENT_FLAG(ent, GLLC_ENT_MODIFIED))
                        continue;

                gllc_ent_build(ent, &block->DBD);

                if (GLLC_ENT_FLAG(ent, GLLC_ENT_GEOMETRY_MODIFIED))
                {
                        int ok = gllc_ent_bbox(ent, block->scale, &bbox_x0, &bbox_y0, &bbox_x1, &bbox_y1);

                        if (!GLLC_ENT_FLAG(ent, GLLC_ENT_INITIAL))
                                gllc_SG_remove(&block->ent_grid, ent);

                        if (ok)
                        {
                                gllc_SG_push(&block->ent_grid, ent, bbox_x0, bbox_y0, bbox_x1, bbox_y1);
                                GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_INITIAL);
                        }
                }

                if (GLLC_ENT_FLAG(ent, GLLC_ENT_SELECTED))
                {
                        double *ver;
                        int ver_count;

                        ver_count = ent->vtable->vertices(ent, block->scale, NULL);

                        if (ver_count < 64)
                                ver = (double *)ver_stack;
                        else
                                ver = malloc(sizeof(double) * 2 * ver_count);

                        if (ver)
                        {
                                ent->vtable->vertices(ent, block->scale, ver);
                                for (i = 0; i < ver_count; i++)
                                {
                                        struct gllc_vertex *vertex = gllc_vertex_create(block, ent, ver[i * 2], ver[i * 2 + 1], i);
                                        if (vertex)
                                        {
                                                push_ent(block, (struct gllc_block_entity *)vertex, 1);
                                                gllc_ent_build((struct gllc_block_entity *)vertex, &block->I_DBD);
                                        }
                                }
                        }

                        if (ver_count >= 64)
                                free(ver);
                }
                else
                {
                        // Удаляем все вершины с прошлой сборки
                        struct gllc_block_entity *I_ent = block->I_ent_head;
                        while (I_ent)
                        {
                                struct gllc_block_entity *next = I_ent->next;
                                if (((struct gllc_vertex *)I_ent)->owner == ent)
                                {
                                        remove_ent(block, I_ent, 1);
                                        gllc_ent_destroy(I_ent);
                                }

                                I_ent = next;
                        }
                }

                GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_GEOMETRY_MODIFIED);
                GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_MODIFIED);
        }
}

void gllc_block_ent_deselect_all(struct gllc_block *block)
{
        assert(block);

        struct gllc_block_entity *ent = block->ent_head;
        struct gllc_block_entity *next;
        for (ent = block->ent_head; ent; ent = ent->next)
        {
                if (GLLC_ENT_FLAG(ent, GLLC_ENT_SELECTED))
                {
                        GLLC_ENT_UNSET_FLAG(ent, GLLC_ENT_SELECTED);
                        GLLC_ENT_SET_FLAG(ent, GLLC_ENT_MODIFIED);
                }
        }

        ent = block->I_ent_head;
        while (ent)
        {
                next = ent->next;
                remove_ent(block, ent, 1);
                gllc_ent_destroy(ent);

                ent = next;
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

        GLLC_ENT_SET_FLAG(ent, GLLC_ENT_SELECTED | GLLC_ENT_MODIFIED);
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
        struct gllc_SG_cell *cell;

        gllc_block_ent_deselect_all(block);

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
                        cell = gllc_SG_cell_at(&block->ent_grid, x, y);
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

                                if (!gllc_ent_bbox(cell->ent[i], block->scale, &bbox1_x0, &bbox1_y0, &bbox1_x1, &bbox1_y1))
                                        continue;

                                if (bbox1_x0 > bbox1_x1)
                                        _swapf(&bbox1_x0, &bbox1_x1);
                                if (bbox1_y0 > bbox1_y1)
                                        _swapf(&bbox1_y0, &bbox1_y1);

                                if (!(bbox_x0 <= bbox1_x0 && bbox_y0 <= bbox1_y0 && bbox_x1 >= bbox1_x1 && bbox_y1 >= bbox1_y1))
                                        continue;

                                if (!GLLC_ENT_FLAG(cell->ent[i], GLLC_ENT_SELECTED))
                                        GLLC_ENT_SET_FLAG(cell->ent[i], GLLC_ENT_SELECTED | GLLC_ENT_MODIFIED);
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
                if (cell->ent[i]->vtable->picked(cell->ent[i], block->scale, x, y))
                {
                        ent = cell->ent[i];
                        break;
                }
        }

        return ent;
}

void gllc_block_on_scale(struct gllc_block *block, double new_scale)
{
        printf("gllc_block_on_scale()\n");

        double bbox_x0, bbox_y0, bbox_x1, bbox_y1;

        if (block->scale == new_scale)
                return;

        int i = 0;
        struct gllc_SG *grid = &block->ent_grid;
        struct gllc_block_entity *ent = block->ent_head;

        size_t cnt = 0;

_again:
        for (; ent; ent = ent->next)
        {
                if (!GLLC_ENT_FLAG(ent, GLLC_ENT_SCREEN_SIZE))
                        continue;

                if (!gllc_ent_bbox(ent, block->scale, &bbox_x0, &bbox_y0, &bbox_x1, &bbox_y1))
                        continue;

                if (bbox_x0 > bbox_x1)
                        _swapf(&bbox_x0, &bbox_x1);
                if (bbox_y0 > bbox_y1)
                        _swapf(&bbox_y0, &bbox_y1);

                gllc_SG_remove(grid, ent);
                gllc_SG_push(grid, ent, bbox_x0, bbox_y0, bbox_x1, bbox_y1);

                cnt++;

                printf("%zu\n", cnt);
        }

        if (i == 0)
        {
                // повторяем для Interactive entities

                i = 1;
                grid = &block->I_ent_grid;
                ent = block->I_ent_head;

                goto _again;
        }

        block->scale = new_scale;
}