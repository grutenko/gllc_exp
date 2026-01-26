#ifndef gllc_drawing_h
#define gllc_drawing_h

#include "gllc_object.h"

#include <stddef.h>

struct gllc_drawing
{
        struct gllc_object __obj;
        struct gllc_block *block_head;
        struct gllc_block *block_tail;
        size_t block_count;
};

struct gllc_drawing *gllc_drawing_create(void);
struct gllc_block *gllc_drw_add_block(struct gllc_drawing *drawing,
                                      const char *name, double dx, double dy);
void gllc_drawing_destroy(struct gllc_drawing *drawing);

#endif