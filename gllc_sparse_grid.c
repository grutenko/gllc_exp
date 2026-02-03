#include "gllc_sparse_grid.h"
#include "gllc_block_entity.h"

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

static int push_ent(struct gllc_SG_cell **grid, uint64_t hash, struct gllc_block_entity *ent)
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
                        return 0;

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
                        return 0;

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

        return 1;
}

static inline void _swap(int *a, int *b)
{
        int t = *a;
        *a = *b;
        *b = t;
}

int gllc_SG_push(struct gllc_SG_cell **grid, struct gllc_block_entity *ent, double bbox_x0, double bbox_y0, double bbox_x1, double bbox_y1)
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

        // Вставляем ссылку на элемент во все ячейки входящие в bbox
        for (x = cx0; x <= cx1; x++)
                for (y = cy0; y <= cy1; y++)
                        if (!push_ent(grid, GLLC_SG_HASH(x, y), ent))
                                return 0;

        return 1;
}

void gllc_SG_remove(struct gllc_SG_cell **grid, struct gllc_block_entity *ent)
{
        assert(grid);

        if (!(*grid))
                return;

        gllc_SG_remove(&(*grid)->left, ent);
        gllc_SG_remove(&(*grid)->right, ent);

        if ((*grid)->ent_size == 0)
                return;

        int i;
        for (i = 0; i < (*grid)->ent_size; i++)
                if ((*grid)->ent[i] == ent)
                        break;

        if (i == (*grid)->ent_size)
                return;

        size_t copy_size = (*grid)->ent_size - 1 - i;
        memmove(&(*grid)->ent[i], &(*grid)->ent[i + 1], copy_size * sizeof(struct gllc_block_entity *));

        (*grid)->ent_size--;
}

static struct gllc_SG_cell *find_cell(struct gllc_SG_cell **grid, uint64_t hash)
{
        struct gllc_SG_cell *p = *grid;

        while (p)
        {
                if (p->hash > hash)
                        p = p->left;
                else if (p->hash < hash)
                        p = p->right;
                else
                        break;
        }

        return p;
}

struct gllc_SG_cell *gllc_SG_cell_at(struct gllc_SG_cell **grid, int x, int y)
{
        return find_cell(grid, GLLC_SG_HASH(x, y));
}

struct gllc_SG_cell *gllc_SG_pick_cell(struct gllc_SG_cell **grid, double x, double y)
{
        int cx0 = ((int)floor(x)) >> GLLC_SG_CELL_SHIFT;
        int cy0 = ((int)floor(y)) >> GLLC_SG_CELL_SHIFT;

        return find_cell(grid, GLLC_SG_HASH(cx0, cy0));
}

void gllc_SG_cleanup(struct gllc_SG_cell **grid)
{
}