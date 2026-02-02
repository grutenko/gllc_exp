#ifndef gllc_sparse_grid_h
#define gllc_sparse_grid_h

#include "stdint.h"

struct gllc_block_entity;

#define GLLC_SG_CELL 64
#define GLLC_SG_CELL_SHIFT 6
#define GLLC_SG_HASH(X_, Y_) (uint64_t)((X_) << 24 | (Y_))

struct gllc_SG_cell
{
        int height;
        uint64_t hash;
        struct gllc_block_entity **ent;
        size_t ent_cap;
        size_t ent_size;
        struct gllc_SG_cell *p;
        struct gllc_SG_cell *left;
        struct gllc_SG_cell *right;
};

int gllc_SG_push(struct gllc_SG_cell **grid, struct gllc_block_entity *ent, double bbox_x0, double bbox_y0, double bbox_x1, double bbox_y1);

void gllc_SG_remove(struct gllc_SG_cell **grid, struct gllc_block_entity *ent);

void gllc_SG_cleanup(struct gllc_SG_cell **grid);

struct gllc_SG_cell *gllc_SG_pick_cell(struct gllc_SG_cell **grid, double x, double y);

#endif