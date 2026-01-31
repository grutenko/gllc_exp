#ifndef gllc_rect_h
#define gllc_rect_h

#include "gllc_block_entity.h"

struct gllc_block;
struct gllc_DE;

struct gllc_rect
{
        struct gllc_block_entity __ent;
        double x;
        double y;
        double width;
        double height;
        double angle;
        int filled;

        struct gllc_DE *DE_bound;
        struct gllc_DE *DE_fill;
};

struct gllc_rect *gllc_rect_create(struct gllc_block *block, double x0, double y0, double width, double height, double angle, int filled);

#endif