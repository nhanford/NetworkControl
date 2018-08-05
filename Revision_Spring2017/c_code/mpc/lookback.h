
#include <linux/types.h>

#ifndef LOOKBACK_H
#define LOOKBACK_H

/*
 * This struct tracks observations made in the past
 */
struct lookback {
	size_t size;
	u64 *elems;

	// Points to most recent element.
	size_t head;
};

void lookback_init(struct lookback *lb, size_t size, u64 elem);
void lookback_release(struct lookback *lb);

// Add an observation to the lookback.
void lookback_add(struct lookback *lb, u64 elem);

// Index an observation, starting with the newest one, and returning a pointer
// to it. Doesn't do bound checks.
static inline u64 *lookback_index(struct lookback *lb, size_t idx)
{
	if (idx > lb->head)
		return &lb->elems[lb->size + lb->head - idx];
	else
		return &lb->elems[lb->head - idx];
}

// The number of observations being tracked.
static inline size_t lookback_size(struct lookback *lb)
{
	return lb->size;
}

#endif /* end of include guard: LOOKBACK_H */
