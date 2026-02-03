#ifndef gllc_vertex_h
#define gllc_vertex_h

#include "gllc_block_entity.h"

// Внутрениий gllc_block_entity для отображения и работы с вершиными выделеных элементов

struct gllc_DE;

struct gllc_vertex
{
        struct gllc_block_entity __ent;
        double x;
        double y;
        struct gllc_DE *DE_fill;
};
 
#endif