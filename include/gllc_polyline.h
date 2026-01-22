#ifndef gllc_polyline_h
#define gllc_polyline_h

#include "gllc_block_entity.h"

struct gllc_block;
struct gllc_draw_ent;

struct gllc_polyline {
  struct gllc_block_entity __ent;
  double *ver;
  size_t ver_cap;
  size_t ver_size;
  int closed;
  struct gllc_draw_ent *draw_bound;
  struct gllc_draw_ent *draw_fill;
};

struct gllc_polyline *gllc_polyline_create(struct gllc_block *block,
                                           int closed);
void gllc_polyline_add_ver(struct gllc_polyline *pline, double x, double y);
#endif