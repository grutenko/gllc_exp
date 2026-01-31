#ifndef gllc_point_h
#define gllc_point_h

#include "gllc_block_entity.h"

struct gllc_block;
struct gllc_DE;

struct gllc_point
{
        struct gllc_block_entity __ent;
        double x;
        double y;
        struct gllc_DE *DE;
};

struct gllc_point *gllc_point_create(struct gllc_block *block, double x, double y);

#endif