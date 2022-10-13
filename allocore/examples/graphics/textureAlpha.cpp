/*
Allocore Example: Texture w/ Alpha Blending

Description:
Example of transparent blending of an image onto a scene using the image's
alpha channel. The alpha channel specifies the level of opaqueness, thus alpha=0
means totally transparent and alpha=1 means totally opaque.

For examples of more OpenGL blending modes, see

	http://www.andersriggelsen.dk/glblendfunc.php

Author:
Lance Putnam, Feb. 2015
*/
#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Texture.hpp"
using namespace al;


class MyApp : public App{
public:

	Mesh mesh;
	Texture tex{256,256};

	MyApp(){

		// Generate a texture with an alpha channel for transparency
		tex.allocate();
		
		tex.assignFromTexCoord<Color>([](float u, float v){
			float px = u*M_2PI;
			float py = v*M_2PI;
			float z = cos(2*px) * cos(5*py) - cos(5*px) * cos(2*py);
			Color c = RGB(1);
			// Make alpha channel 1 where function is zero using Gaussian
			c.a = exp(-16*z*z);
			return c;
		});

		// Load an image having an alpha channel
		//Image img("myImage.png");
		//tex.allocate(img.array());

		mesh.primitive(Graphics::TRIANGLES);
		mesh.vertex(-1,-1,-1); mesh.color(RGB(1,0,0));
		mesh.vertex( 1,-1,-1); mesh.color(RGB(1,1,0));
		mesh.vertex( 0, 1,-1); mesh.color(RGB(0,1,1));

		nav().pullBack(4);
		initWindow();
	}

	void onDraw(Graphics& g) override {
		
		// Draw all scene objects first
		g.draw(mesh);
		
		// Here we activate transparent blending. This mixes source and
		// destination colors according to the source's alpha value.
		g.blendTrans();

		// Render texture on a rectangle in world space ...
		g.color(RGB(1)); // ensure our quad is white
		tex.quad(g, 2,2,-1,-1);

		// or render texture onto the viewport in normalized device coordinates
		//tex.quadViewport(g);

		// Turn off blending
		g.blendOff();
	}

};

int main(){
	MyApp().start();
}

