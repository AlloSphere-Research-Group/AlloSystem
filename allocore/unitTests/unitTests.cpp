
#include "allocore/al_Allocore.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

using namespace al;

SearchPaths searchpaths;
SearchPaths& getSearchPaths() { return searchpaths; }

//#ifndef ALLOCORE_TESTS_NO_AUDIO
//RUNTEST(IOAudioIO);
//RUNTEST(AudioScene);
//#endif

//RUNTEST(Ambisonics);

//#ifndef ALLOCORE_TESTS_NO_GUI
//// This test should always be run last since it calls exit()
//// This test will not run on headless machines.
//printf("IOWindow .... (calls exit() internally)\n");
//utIOWindowGL();
//#endif


int main (int argc, char * const argv[]) {
	searchpaths.addAppPaths(argc, argv);
	searchpaths.addSearchPath(searchpaths.appPath() + "../share");

	int result = Catch::Session().run( argc, argv );

	// Empirical tests; leave commented
	// These are tests that require some kind of observation to validate.

//	utAsset();
//	utGraphicsDraw();

	return result;
}


