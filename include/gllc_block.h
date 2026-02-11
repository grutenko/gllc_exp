#ifndef gllc_block_h
#define gllc_block_h

#include "gllc_block_entity.h"
#include "gllc_draw_buffer.h"
#include "gllc_object.h"
#include "gllc_sparse_grid.h"

struct gllc_layer;
struct gllc_drawing;
struct gllc_polyline;
struct gllc_rect;
struct gllc_point;

struct gllc_block
{
        struct gllc_object __obj;
        char name[64];
        double dx;
        double dy;
        // Текущий scale окна. Используется для правильного поиска границ для GLLC_ENT_SCREEN_SIZE элементов
        double scale;
        struct gllc_block_entity_props props;
        struct gllc_drawing *drawing;
        struct gllc_DBD DBD;
        struct gllc_block_entity *ent_head;
        struct gllc_block_entity *ent_tail;
        size_t ent_count;
        struct gllc_SG ent_grid;
        // Структуры для управления интерактивными элементами.
        // По факту работаем с ними также как и с обычными, но они полностью отделены друг от друга
        // пока используетсят только для точек вешин выделеных элементов
        struct gllc_DBD I_DBD;
        struct gllc_block_entity *I_ent_head;
        struct gllc_block_entity *I_ent_tail;
        size_t I_ent_count;
        struct gllc_SG I_ent_grid;
        struct gllc_layer *layer_head;
        struct gllc_layer *layer_tail;
        size_t layer_count;
        struct gllc_block *next;
        struct gllc_block *prev;
};

struct gllc_block *gllc_block_create(struct gllc_drawing *drawing, const char *name, double dx, double dy);

struct gllc_polyline *gllc_block_add_polyline(struct gllc_block *block, int closed, int filled);

struct gllc_rect *gllc_block_add_rect(struct gllc_block *block, double x, double y, double width, double height, double angle, int filled);

void gllc_block_update(struct gllc_block *block);

void gllc_block_destroy(struct gllc_block *block);

struct gllc_block_entity *gllc_block_get_first_ent(struct gllc_block *block);

struct gllc_circle *gllc_block_add_circle(struct gllc_block *block, double x, double y, double radius, int filled);

void gllc_block_remove_ent(struct gllc_block *block, struct gllc_block_entity *ent);

struct gllc_point *gllc_block_add_point(struct gllc_block *block, double x, double y);

struct gllc_block_entity *gllc_block_pick_ent(struct gllc_block *block, double x, double y);

void gllc_block_ent_select(struct gllc_block *block, struct gllc_block_entity *ent, int exlusive);

struct gllc_block_entity *gllc_block_select_ent_by_point(struct gllc_block *block, double x, double y, int exlusive);

void gllc_block_ent_deselect_all(struct gllc_block *block);

void gllc_block_ent_deselect(struct gllc_block *block, struct gllc_block_entity *ent);

void gllc_block_ent_select_by_bbox(struct gllc_block *block, double bbox_x0, double bbox_y0, double bbox_x1, double bbox_y1);

void gllc_block_on_scale(struct gllc_block *block, double new_scale);

#endif