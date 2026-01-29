#ifndef gllc_block_h
#define gllc_block_h

#include "gllc_block_entity.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"

struct gllc_layer;
struct gllc_drawing;
struct gllc_polyline;
struct gllc_rect;
struct gllc_QTree_node;

struct gllc_block
{
        struct gllc_object __obj;
        char name[64];
        double dx;
        double dy;
        struct gllc_block_entity_props props;
        struct gllc_drawing *drawing;
        struct gllc_DBD DBD;
        struct gllc_block_entity *ent_head;
        struct gllc_block_entity *ent_tail;
        size_t ent_count;
        struct gllc_QTree_node *Q_tree;
        struct gllc_layer *layer_head;
        struct gllc_layer *layer_tail;
        size_t layer_count;
        struct gllc_block *next;
        struct gllc_block *prev;
};

struct gllc_block *gllc_block_create(struct gllc_drawing *drawing, const char *name, double dx, double dy);

struct gllc_polyline *gllc_block_add_polyline(struct gllc_block *block, int closed, int filled);

struct gllc_rect *gllc_block_add_rect(struct gllc_block *block, double x0, double y0, double x1, double y1, int filled);

void gllc_block_update(struct gllc_block *block);

void gllc_block_destroy(struct gllc_block *block);

struct gllc_block_entity *gllc_block_get_first_ent(struct gllc_block *block);

struct gllc_circle *gllc_block_add_circle(struct gllc_block *block, double x, double y, double radius, int filled);

void gllc_block_remove_ent(struct gllc_block *block, struct gllc_block_entity *ent);

#endif