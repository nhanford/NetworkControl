
#include <linux/slab.h>

#include "lookback.h"


int lookback_init(struct lookback *lb, size_t size, s64 elem)
{
	int i;

	lb->size = size;
	lb->elems = kmalloc(size * sizeof(s64), GFP_KERNEL);
	lb->head = 0;

	if (lb->elems == NULL)
		goto exit_failure;

	for (i = 0; i < size; i++)
		lb->elems[i] = elem;

	return 0;

exit_failure:
	return 1;
}

void lookback_release(struct lookback *lb)
{
	kfree(lb->elems);
}

void lookback_add(struct lookback *lb, s64 elem)
{
	lb->head = (lb->head + 1) % lb->size;
	lb->elems[lb->head] = elem;
}
