#ifndef gllc_rect_h
#define gllc_rect_h

#include "gllc_block_entity.h"

struct gllc_block;
struct gllc_DE;

struct gllc_rect
{
        struct gllc_block_entity __ent;
        double x0;
        double y0;
        double x1;
        double y1;

        struct gllc_DE *DE_bound;
};

struct gllc_rect *gllc_rect_create(struct gllc_block *block, double x0, double y0, double x1, double y1);

#endif