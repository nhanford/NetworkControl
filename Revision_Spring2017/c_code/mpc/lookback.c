
#include <linux/slab.h>

#include "lookback.h"


void lookback_init(struct lookback *lb, size_t size, real elem)
{
    int i;

    lb->size = size;
    lb->elems = kmalloc(size * sizeof(real), GFP_KERNEL);
    lb->head = 0;

    for(i = 0; i < size; i++)
        lb->elems[i] = elem;
}

void lookback_release(struct lookback *lb)
{
    kfree(lb->elems);
}

void lookback_add(struct lookback *lb, real elem)
{
    lb->head = (lb->head + 1) % lb->size;
    lb->elems[lb->head] = elem;
}
