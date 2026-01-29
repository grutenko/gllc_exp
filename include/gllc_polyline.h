#ifndef gllc_polyline_h
#define gllc_polyline_h

#include "gllc_block_entity.h"
#include "gllc_object.h"

struct gllc_block;
struct gllc_draw_ent;

enum
{
        GLLC_POLYLINE_BULGE = 0,
        GLLC_POLYLINE_QUAD = 1,
        GLLC_POLYLINE_CUBIC = 2,
        GLLC_POLYLINE_BEZIER = 3,
        GLLC_POLYLINE_SPLINE = 4,
        GLLC_POLYLINE_ROUND = 5,
        GLLC_POLYLINE_LINQUAD = 6
};

struct gllc_polyline_vertex
{
        struct gllc_object __obj;
        double x;
        double y;
};

struct gllc_polyline
{
        struct gllc_block_entity __ent;
        struct gllc_polyline_vertex *ver;
        size_t ver_cap;
        size_t ver_size;
        struct gllc_DE *DE_bound;
        struct gllc_DE *DE_fill;
        int closed;
        int filled;
        int fit_type;
};

struct gllc_polyline *gllc_polyline_create(struct gllc_block *block, int closed, int filled);

void gllc_polyline_add_ver(struct gllc_polyline *pline, double x, double y);

void gllc_polyline_build(const struct gllc_polyline_vertex *V, size_t V_count, struct gllc_DE *DE_bound, struct gllc_DE *DE_fill, float *color, float *fcolor, int closed, int filled);

#endif