/*
Allocore Example: FBO

Description:
This demonstrates how to use an FBO
MipMaps:

FBOs won't generate the mip maps automatically
If texture filterMin is set to a MIPMAP option, then the texture will need to have mipmaps generated manually (after the FBO is unbound), using tex.generateMipmap();

Author:
Graham Wakefield, 2012
Reworked into al::App by Lance Putnam, 2021
*/

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_FBO.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/math/al_Random.hpp"
using namespace al;


struct MyApp : App {

	RBO rbo;
	FBO fbo;
	Texture fbotex{256, 256};

	MyApp(){
		nav().pullBack(4);
		initWindow();
	}

	void onCreate(const ViewpointWindow& win) override {
		// both depth and color attachees must be valid on the GPU before use:
		rbo.resize(fbotex.width(), fbotex.height());
		//fbotex.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
		fbotex.validate();

		fbo.attachRBO(rbo, FBO::DEPTH_ATTACHMENT);
		fbo.attachTexture2D(fbotex.id(), FBO::COLOR_ATTACHMENT0);
		printf("fbo status %s\n", fbo.statusString());
	}

	void onDraw(Graphics& g) override {

		fbo.begin();
			// capture green-world to texture:
			g.viewport(0, 0, fbotex.width(), fbotex.height());
			g.clearColor(0, 0.5, 0, 0);
			g.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);

			g.projection(Matrix4d::perspective(45, 1, 0.1, 10));
			g.modelView(Matrix4d::identity());

			g.begin(g.TRIANGLES);
			g.color(1, 1, 0);
				g.vertex(rnd::uniformS()*0.5, rnd::uniformS()*0.5, -5);
				g.vertex(rnd::uniformS()*0.5, rnd::uniformS()*0.5, -5);
				g.vertex(rnd::uniformS()*0.5, rnd::uniformS()*0.5, -5);
			g.end();
		fbo.end();

		// generation of Mipmaps must be done manually for FBO-bound textures:
		// (to see why, comment this code out and then make the window very small).
		//fbotex.generateMipmap();

		// show in blue-world:
		g.viewport(0, 0, window().width(), window().height());
		g.clearColor(0, 0, 0.5, 0);
		g.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);

		g.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
		g.modelView(Matrix4d::identity());
		fbotex.quad(g, 0.8, 0.8, 0.1, 0.1);
	}
};

int main(){
	MyApp().start();
}
