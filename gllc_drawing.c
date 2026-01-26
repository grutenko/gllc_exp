#include "gllc_drawing.h"
#include "gllc_block.h"
#include "gllc_object.h"
#include <stdlib.h>
#include <string.h>

static const struct gllc_prop_def g_prop_def[] = {{-1, -1, 0, 0, 0}};

static const struct gllc_prop_def *g_props_def[] = {g_prop_def, 0};

struct gllc_drawing *gllc_drawing_create(void)
{
        struct gllc_drawing *drawing = malloc(sizeof(struct gllc_drawing));
        if (drawing)
        {
                memset(drawing, 0, sizeof(struct gllc_drawing));
                drawing->__obj.prop_def = g_props_def;
        }
        return drawing;
}

static void push_block(struct gllc_drawing *drawing, struct gllc_block *block)
{
        block->prev = drawing->block_tail;
        block->next = NULL;
        if (drawing->block_tail)
                drawing->block_tail->next = block;
        else
                drawing->block_head = block;
        drawing->block_tail = block;
        drawing->block_count++;
}

struct gllc_block *gllc_drw_add_block(struct gllc_drawing *drawing,
                                      const char *name, double dx, double dy)
{
        struct gllc_block *block = gllc_block_create(drawing, name, dx, dy);
        if (block)
        {
                push_block(drawing, block);
        }
        return block;
}

void gllc_drawing_destroy(struct gllc_drawing *drawing)
{
        struct gllc_block *block = drawing->block_head;
        while (block)
        {
                struct gllc_block *next = block->next;
                gllc_block_destroy(block);
                block = next;
        }
        free(drawing);
}