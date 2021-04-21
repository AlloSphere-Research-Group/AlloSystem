/*
Allocore Example: Model File

Description:
Loads a 3D model file into a mesh and renders it.

Author:
Lance Putnam, April 2021
*/

#include "allocore/io/al_App.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/graphics/al_Asset.hpp"
using namespace al;

class MyApp : public App{
public:

	Mesh model;

	MyApp(){
		std::string modelDir = "allocore/share/models/";
		if(!File::searchBack(modelDir)){
			printf("Error: Failed to find models directory\n");
			exit(-1);
		}

		auto * scene = Scene::import(modelDir + "ant.ply");
		if(!scene){
			printf("Error: Failed to load model file\n");
			exit(-1);
		}

		// Merge all data from model file into mesh
		scene->meshAll(model);

		// We have no idea what the distance units are, so fit into unit cube
		model.fitToCube();

		// If you prefer flat shading...
		//model.decompress().generateNormals();

		nav().pullBack(4);
		initWindow();
	}

	void onDraw(Graphics& g) override {
		g.light().pos(100, 1000, 100);
		g.draw(model);
	}
};


int main(){
	MyApp().start();
}
