#include "utAllocore.h"

#include "allocore/graphics/al_Asset.hpp"

int utAsset() {
	
	
	Scene * scene = Scene::import(getSearchPaths().find("ducky.obj").filepath());
	scene->dump();
		
	return 0;
}
