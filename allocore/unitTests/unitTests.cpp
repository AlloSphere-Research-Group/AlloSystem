#include "utAllocore.h"

SearchPaths searchpaths;
SearchPaths& getSearchPaths() { return searchpaths; }

int main (int argc, char * const argv[]) {
	searchpaths.addAppPaths(argc, argv);
	searchpaths.addSearchPath(searchpaths.appPath() + "../share");

	// Logical tests; leave uncommented
	// These tests are purely logical, they should not print anything out to
	// the console (as it interferes with assertion error messages) or start any
	// processes that require user intervention to proceed or validate the test.

	// This macro runs the unit test and prints out status info
	// 'Name' should match the name of the unit test, utName.
	#define RUNTEST(Name)\
		printf("%s ", #Name);\
		ut##Name();\
		for(size_t i=0; i<32-strlen(#Name); ++i) printf(".");\
		printf(" pass\n")

	RUNTEST(Math);
	RUNTEST(MathSpherical);
	RUNTEST(Types);
	RUNTEST(TypesConversion);
	RUNTEST(Spatial);
	RUNTEST(System);
	RUNTEST(ProtocolOSC);
	RUNTEST(ProtocolSerialize);

	RUNTEST(IOSocket);
	RUNTEST(File);
	RUNTEST(Thread);

	RUNTEST(GraphicsMesh);
	
	// This test should always be run last since it calls exit()
	printf("IOWindow\n");
	utIOWindowGL();


	// Empirical tests; leave commented
	// These are tests that require some kind of observation to validate.

//	utAsset();
//	utGraphicsDraw();
//	utIOAudioIO();

	return 0;
}


