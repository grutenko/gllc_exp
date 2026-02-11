#ifndef gllc_sparse_grid_h
#define gllc_sparse_grid_h

#include "stdint.h"

struct gllc_block_entity;

#define GLLC_SG_CELL 128
#define GLLC_SG_CELL_SHIFT 7
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

struct gllc_SG_lookup
{
        struct gllc_block_entity *ent;
        struct gllc_SG_cell *cell;
        size_t pos;
};

struct gllc_SG
{
        struct gllc_SG_lookup *lookup_table;
        size_t lookup_table_cap;
        size_t lookup_table_cnt;
        struct gllc_SG_cell *root;
};

int gllc_SG_push(struct gllc_SG *grid, struct gllc_block_entity *ent, double bbox_x0, double bbox_y0, double bbox_x1, double bbox_y1);

void gllc_SG_remove(struct gllc_SG *grid, struct gllc_block_entity *ent);

void gllc_SG_cleanup(struct gllc_SG *grid);

struct gllc_SG_cell *gllc_SG_pick_cell(struct gllc_SG *grid, double x, double y);

struct gllc_SG_cell *gllc_SG_cell_at(struct gllc_SG *grid, int x, int y);

#endif