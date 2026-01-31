#ifndef gllc_circle_h
#define gllc_circle_h

#include "gllc_block_entity.h"

struct gllc_DE;
struct gllc_block;

struct gllc_circle
{
        struct gllc_block_entity __ent;
        double x;
        double y;
        double radius;

        struct gllc_DE *DE_bound;
        struct gllc_DE *DE_fill;
};

struct gllc_circle *gllc_circle_create(struct gllc_block *block, double x, double y, double radius, int filled);

#endif