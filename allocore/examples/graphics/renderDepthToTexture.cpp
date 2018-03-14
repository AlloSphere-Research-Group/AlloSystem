/*
Allocore Example: Render Depth To Texture

Description:
This demonstrates how to render the depth buffer of a scene into a texture using a frame buffer object (FBO).

Author:
Lance Putnam, 2015
*/

#include "allocore/io/al_App.hpp"
using namespace al;

class MyApp : public App{
public:
	Mesh mesh;
	FBO fbo;			// Frame buffer object; this is our render target
	Texture texDepth;	// Texture to write depth buffer to

	MyApp(){
		addCylinder(mesh, 0.02);
		nav().pullBack(1.1);
		initWindow();
	}
	
	void onResize(const ViewpointWindow& win, int w, int h){

		// Configure texture on GPU
		texDepth.format(Graphics::DEPTH_COMPONENT);
		texDepth.type(Graphics::UBYTE);
		texDepth.resize(w/4, h/4); // render at quarter size
		texDepth.submit(); // ensure texture gets configured on GPU

		fbo.bind();

		// These calls are sometimes needed if we do not have a color buffer attached
		graphics().drawBuffer(Graphics::NONE);
		graphics().readBuffer(Graphics::NONE);

		// Attach the texture to the FBO
		fbo.attachTexture2D(texDepth.id(), FBO::DEPTH_ATTACHMENT);

		fbo.unbind();
	}

	void onDraw(Graphics& g, const Viewpoint& v){

		// To render our scene to the FBO, we must first bind it
		fbo.bind();
			g.viewport(0, 0, texDepth.width(), texDepth.height());
			g.clear(Graphics::DEPTH_BUFFER_BIT);

			//Disable color rendering; we only want to write to the z-buffer
			g.colorMask(false);

			// Draw our scene
			g.draw(mesh);
		fbo.unbind();

		// Show depth buffer texture
		g.viewport(0, 0, v.viewport().w, v.viewport().h);
		g.colorMask(true);
		texDepth.quadViewport(g);
	}
};

int main(){
	MyApp().start();
}
