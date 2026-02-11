#include "gllc_sparse_grid.h"
#include "gllc_block_entity.h"
#include "include/gllc_sparse_grid.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AVL_HEIGHT(node_) ()
#define AVL_LL(node_)
#define AVL_RR(node_)
#define AVL_LR(node_)
#define AVL_RL(node_)

static struct gllc_SG_cell *push_ent(struct gllc_SG_cell **grid, uint64_t hash, struct gllc_block_entity *ent, size_t *out_pos)
{
        int pos;
        int i;
        struct gllc_SG_cell **p = grid;
        struct gllc_SG_cell *prev = NULL;

        while (*p)
        {
                prev = *p;
                if ((*p)->hash > hash)
                        p = &(*p)->left;
                else if ((*p)->hash < hash)
                        p = &(*p)->right;
                else
                        break;
        }

        if (!(*p))
        {
                *p = (struct gllc_SG_cell *)malloc(sizeof(struct gllc_SG_cell));
                if (!(*p))
                        return NULL;

                (*p)->height = 0;
                (*p)->hash = hash;
                (*p)->left = (void *)0ULL;
                (*p)->right = (void *)0ULL;
                (*p)->ent = (void *)0ULL;
                (*p)->ent_cap = 0ULL;
                (*p)->ent_size = 0ULL;
        }

        // TODO: AVL balance

        if ((*p)->ent_cap <= (*p)->ent_size + 1)
        {
                size_t new_cap = (*p)->ent_cap ? (*p)->ent_cap * 2 : 8;
                struct gllc_block_entity **new_ent = realloc((*p)->ent, sizeof(struct gllc_block_entity *) * new_cap);
                if (!new_ent)
                        return NULL;

                (*p)->ent = new_ent;
                (*p)->ent_cap = new_cap;
        }

        for (pos = 0; pos < (*p)->ent_size; pos++)
        {
                if ((*p)->ent[pos]->order <= ent->order)
                        break;
        }

        size_t copy_size = (*p)->ent_size - pos;
        memmove(&(*p)->ent[pos + 1], &(*p)->ent[pos], copy_size * sizeof(struct gllc_block_entity *));

        (*p)->ent[pos] = ent;
        (*p)->ent_size++;
        *out_pos = pos;

        return *p;
}

static inline void _swap(int *a, int *b)
{
        int t = *a;
        *a = *b;
        *b = t;
}

static int push_lookup(struct gllc_SG *grid, struct gllc_SG_cell *cell, struct gllc_block_entity *ent, size_t pos)
{
        if (grid->lookup_table_cap <= grid->lookup_table_cnt + 1)
        {
                size_t new_size = grid->lookup_table_cap ? grid->lookup_table_cap * 2 : 128;
                struct gllc_SG_lookup *new_lookup = realloc(grid->lookup_table, new_size * sizeof(struct gllc_SG_lookup));
                if (!new_lookup)
                        return 0;

                grid->lookup_table = new_lookup;
                grid->lookup_table_cap = new_size;
        }

        grid->lookup_table[grid->lookup_table_cnt].cell = cell;
        grid->lookup_table[grid->lookup_table_cnt].ent = ent;
        grid->lookup_table[grid->lookup_table_cnt].pos = pos;
        grid->lookup_table_cnt++;

        return 1;
}

int gllc_SG_push(struct gllc_SG *grid, struct gllc_block_entity *ent, double bbox_x0, double bbox_y0, double bbox_x1, double bbox_y1)
{
        int cx0 = ((int)floor(bbox_x0)) >> GLLC_SG_CELL_SHIFT;
        int cy0 = ((int)floor(bbox_y0)) >> GLLC_SG_CELL_SHIFT;
        int cx1 = ((int)floor(bbox_x1)) >> GLLC_SG_CELL_SHIFT;
        int cy1 = ((int)floor(bbox_y1)) >> GLLC_SG_CELL_SHIFT;

        if (cx0 > cx1)
                _swap(&cx0, &cx1);
        if (cy0 > cy1)
                _swap(&cy0, &cy1);

        int x, y;
        size_t cell_pos;
        struct gllc_SG_cell *cell;

        // Вставляем ссылку на элемент во все ячейки входящие в bbox
        for (x = cx0; x <= cx1; x++)
        {
                for (y = cy0; y <= cy1; y++)
                {
                        cell = push_ent(&grid->root, GLLC_SG_HASH(x, y), ent, &cell_pos);
                        if (!cell)
                                return 0;
                        if (!push_lookup(grid, cell, ent, cell_pos))
                                return 0;
                }
        }

        return 1;
}

void gllc_SG_remove(struct gllc_SG *grid, struct gllc_block_entity *ent)
{
        assert(grid);

        if (!grid->root)
                return;

        struct gllc_SG_cell *cell;
        size_t cell_pos, copy_size, lookup_pos;

_again:

        cell = NULL;

        int i;
        for (i = 0; i < grid->lookup_table_cnt; i++)
        {
                if (grid->lookup_table[i].ent == ent)
                {
                        cell = grid->lookup_table[i].cell;
                        cell_pos = grid->lookup_table[i].pos;
                        lookup_pos = i;
                        break;
                }
        }

        if (!cell)
                return;

        copy_size = grid->lookup_table_cnt - lookup_pos - 1;
        memmove(&grid->lookup_table[lookup_pos], &grid->lookup_table[lookup_pos + 1], copy_size * sizeof(struct gllc_SG_lookup));
        grid->lookup_table_cnt--;

        assert(cell_pos < cell->ent_size);

        if (cell_pos < cell->ent_size - 1)
        {
                // Перемещаем данные и заменяем индексы только если после удаляемого еще есть элементы ...
                copy_size = cell->ent_size - cell_pos - 1;
                memmove(&cell->ent[cell_pos], &cell->ent[cell_pos + 1], copy_size * sizeof(struct gllc_block_entity *));

                for (i = 0; i < grid->lookup_table_cnt; i++)
                {
                        if (grid->lookup_table[i].cell == cell && grid->lookup_table[i].pos > cell_pos)
                                grid->lookup_table[i].pos--;
                }
        }

        // ... Иначе просто уменьшаем размер
        cell->ent_size--;

        goto _again;
}

size_t depth = 0;

static struct gllc_SG_cell *find_cell(struct gllc_SG_cell **grid, uint64_t hash)
{
        struct gllc_SG_cell *p = *grid;

        size_t local_depth = 0;
        while (p)
        {
                if (p->hash > hash)
                        p = p->left;
                else if (p->hash < hash)
                        p = p->right;
                else
                        break;

                local_depth++;
        }

        if(local_depth > depth)
        {
                depth = local_depth;
                printf("DEPTH: %zu\n", depth);
        }

        return p;
}

struct gllc_SG_cell *gllc_SG_cell_at(struct gllc_SG *grid, int x, int y)
{
        return find_cell(&grid->root, GLLC_SG_HASH(x, y));
}

struct gllc_SG_cell *gllc_SG_pick_cell(struct gllc_SG *grid, double x, double y)
{
        int cx0 = ((int)floor(x)) >> GLLC_SG_CELL_SHIFT;
        int cy0 = ((int)floor(y)) >> GLLC_SG_CELL_SHIFT;

        return find_cell(&grid->root, GLLC_SG_HASH(cx0, cy0));
}

void gllc_SG_cleanup(struct gllc_SG *grid)
{
}