#include "utAllocore.h"

SearchPaths searchpaths;

SearchPaths& getSearchPaths() { return searchpaths; }

int main (int argc, char * const argv[]) {
	searchpaths.addAppPaths(argc, argv);
	searchpaths.addSearchPath(searchpaths.appPath() + "../share");

//	utAsset();
	
	utMath();
	utTypes();
	utTypesConversion();
//	utSpatial();
//	utSystem();
	utProtocolOSC();
//	utProtocolSerialize();

//	utIOAudioIO();
//	utIOWindowGL();
//	utIOSocket();
//	utFile();
	utThread();

//	utGraphicsDraw();
	utGraphicsMesh();

//	int size = 14;
//	for (int z=-64; z<64; z++) {
//		printf("%d %d %d\n", z, (z & size), !!(z & size));
//	}

//	printf("%d\n", sizeof(Frustumd));
	return 0;
}


