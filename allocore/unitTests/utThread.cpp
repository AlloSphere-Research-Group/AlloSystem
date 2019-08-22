#include "utAllocore.h"

int utThread() {

	//UT_PRINTF("system: thread\n");

	struct Functor {
		int& x;
		void operator()(){ x = 1; }
	};

	// C++ style
	{
		int x = 0;
		Functor f{x};
		Thread t(f);
		t.join();
		assert(1 == x);
	}

	// Assignment
	{
		int x=0;
		Functor f{x};

		Thread * t2 = new Thread;
		t2->joinOnDestroy(false);
		{
			Thread t1;
			t1.joinOnDestroy(true);
			t1.start(f);
			*t2 = t1;
			// t1 will now get destructed, so t2 better be well-formed
		}

		x=0;
		t2->start(f);
		delete t2; // t2 should join on destruction, since t1 did
		assert(1 == x);
	}

	return 0;
}
