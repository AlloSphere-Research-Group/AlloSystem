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
	
	utAsset();
	
	utMath();
	utTypes();
	utTypesConversion();
	utSpatial();	// TODO: fails
	utSystem();
	utProtocolOSC();
	utProtocolSerialize();

	utIOSocket();
	utFile();
	utThread();

	utGraphicsMesh();

	// Empirical tests; leave commented
	// These are tests that require some kind of observation to validate.

//	utGraphicsDraw();
//	utIOAudioIO();
//	utIOWindowGL();

	return 0;
}


