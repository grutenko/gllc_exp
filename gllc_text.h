#ifndef gllc_text_h
#define gllc_text_h

#include "gllc_block_entity.h"

struct gllc_block;

struct gllc_text {
  struct gllc_block_entity __ent;
  double x;
  double y;
  double width;
  double height;
  double angle;
  char *text;
  size_t text_size;
};

struct gllc_text *gllc_text_create(struct gllc_block *block, const char *text,
                                   double x, double y, double width,
                                   double height, double angle);

#endif