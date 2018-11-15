
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "scaled_shorthand.h"


int main(int argc, char *argv[])
{
	scaled x = SFI(100, 20);
	scaled y = SFI(100, 30);

	assert(scaled_lt(x, y));

	assert(SEQ(SD(ONE, TWO), SFI(1, -1)));

	assert(SEQ(SD(ONE, SFI(4, 0)), SFI(1, -2)));

	assert(SEQ(SM(ZERO, ONE), ZERO));
	assert(SEQ(SM(ONE, ZERO), ZERO));
	assert(SEQ(SD(ZERO, ONE), ZERO));
	assert(0 == STI(SFI(0, 0)));

	// 1/4 < 1/3 < 1/2
	assert(SLT(SD(ONE, SFI(3, 0)), SFI(1, -1)));
	assert(SLT(SFI(1, -2), SD(ONE, SFI(3, 0))));

	assert(SEQ(SS(ONE, ONE), ZERO));

	assert(!SEQ(SS(TWO, ONE), ZERO));

	for (int64_t i = -100; i <= 100; i++) {
		for (int64_t j = -30; j <= 30; j++) {
			//printf("%ld, %ld\n", i, j);

			assert(SEQ(SA(SFI(i, 0), SFI(j, 0)),
				SFI(i + j, 0)));

			assert(SEQ(SS(SFI(i, 0), SFI(j, 0)),
				SFI(i - j, 0)));

			assert(SEQ(SM(SFI(i, 0), SFI(j, 0)),
				SFI(i * j, 0)));

			if (i >= 0 && j >= 0)
				assert((i << j) == STI(SFI(i, j)));
			else if (i < 0 && j < 0)
				assert(-(-i >> -j) == STI(SFI(i, j)));
			else if (i < 0 && j >= 0)
				assert(-(-i << j) == STI(SFI(i, j)));
			else if (i >= 0 && j < 0)
				assert((i >> -j) == STI(SFI(i, j)));
		}
	}

	assert(SEQ(scaled_min(ONE, TWO), ONE));
	assert(SEQ(scaled_max(ONE, TWO), TWO));

	return 0;
}
