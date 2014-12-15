#include "utAllocore.h"

#include "allocore/graphics/al_Asset.hpp"

int utAsset() {
	SearchPaths searchPaths;
	searchPaths.addAppPaths();
	Scene * scene = Scene::import(searchPaths.find("ducky.obj").filepath());
	scene->dump();
	return 0;
}
