/*
Allocore Example: Textured Model File

Description:
Loads a 3D model file into a mesh and renders it along with its texture map.

Author:
Lance Putnam, April 2021
*/

#include "allocore/io/al_App.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_Image.hpp"
#include "allocore/graphics/al_Texture.hpp"
using namespace al;

class MyApp : public App{
public:

	Mesh model;
	Texture tex;

	MyApp(){
		std::string modelDir = "allocore/share/models/";
		if(!File::searchBack(modelDir)){
			printf("Error: Failed to find models directory\n");
			exit(-1);
		}

		if(!model.load(modelDir + "spot.obj")){
			printf("Error: Failed to load model file\n");
			exit(-1);
		}

		// We have no idea what the distance units are, so fit into unit cube
		model.fitToCube();

		// If you prefer flat shading...
		//model.decompress();

		// Generate normals if they weren't in the file
		if(model.normals().empty()) model.generateNormals();

		// Load the texture map
		Image image;
		if(image.load(modelDir + "spot.png")){
			tex.allocate(image.array());
		} else {
			printf("Error: Failed to load texture file\n");
			exit(-1);
		}

		nav().pullBack(4);
		initWindow();
	}

	void onDraw(Graphics& g) override {
		g.light().pos(100, 1000, 100);
		tex.bind();
		g.draw(model);
		tex.unbind();
	}
};


int main(){
	MyApp().start();
}
