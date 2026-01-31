#ifndef gllc_QTree_h
#define gllc_QTree_h

#include <stddef.h>
#define GLLC_QTREE_MIN_SIZE 64

struct gllc_block_entity;

struct gllc_QTree_node
{
        struct gllc_QTree_node *p;
        int x;
        int y;
        int size;
        struct gllc_QTree_node *childs[4];
        struct gllc_block_entity *ents;
        size_t ents_cap;
        size_t ents_size;
};

int gllc_QTree_push(struct gllc_QTree_node **root, struct gllc_block_entity *ent);

void gllc_QTree_remove(struct gllc_QTree_node **root, struct gllc_block_entity *ent);

void gllc_QTree_destroy(struct gllc_QTree_node **root);

#endif