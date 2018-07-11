
#include <linux/slab.h>

#include "lookback.h"


void lookback_init(struct lookback *lb, size_t size, u32 elem)
{
    int i;

    lb->size = size;
    lb->elems = kmalloc(size * sizeof(u32), GFP_KERNEL);
    lb->head = 0;

    for(i = 0; i < size; i++)
        lb->elems[i] = elem;
}

void lookback_release(struct lookback *lb)
{
    kfree(lb->elems);
}

void lookback_add(struct lookback *lb, u32 elem)
{
    lb->head = (lb->head + 1) % lb->size;
    lb->elems[lb->head] = elem;
}