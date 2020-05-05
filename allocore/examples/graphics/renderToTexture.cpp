/*
Allocore Example: Render To Texture

Description:
This demonstrates how to render a scene into a texture using a frame buffer object (FBO).
Rendering the scene to a texture requires an FBO with a texture attached for the color buffer and a render buffer object (RBO) attached for the depth buffer.

Author:
Lance Putnam, 2015
*/


#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_FBO.hpp"
#include "allocore/graphics/al_Texture.hpp"
using namespace al;

class MyApp : public App{
public:
	Mesh mesh;
	FBO fbo;		// Frame buffer object; this is our render target
	RBO rbo;		// Render buffer object for depth buffer
	Texture tex;	// Texture for color buffer

	MyApp(){
		int Nv = addSphere(mesh);
		for(int i=0; i<Nv; ++i){
			mesh.color(HSV(float(i)/Nv*0.3+0.2,0.5,1));
		}
		nav().pullBack(4);
		nav().faceToward(Vec3f(-2,-1,-1));
		initWindow();
	}

	void onResize(const ViewpointWindow& win, int w, int h){
		// Note: all attachments (textures, RBOs, etc.) to the FBO must have the
		// same width and height.

		// Configure texture on GPU
		tex.format(Graphics::RGB);
		tex.type(Graphics::UBYTE);
		tex.resize(w,h);
		tex.submit(); // ensure texture gets configured on GPU

		// Configure render buffer object on GPU
		rbo.resize(w,h);

		// Finally, attach color texture and depth RBO to FBO
		fbo.attachTexture2D(tex.id());
		fbo.attachRBO(rbo, FBO::DEPTH_ATTACHMENT);
	}

	void onDraw(Graphics& g){

		// To render our scene to the FBO, we must first bind it
		fbo.bind();
			// Clear FBO
			g.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);

			// Render our scene as we normally would
			g.draw(mesh);

		// When done rendering our scene to the FBO, we must unbind it
		fbo.unbind();

		// To prove that this all worked, we render the FBO's color texture
		tex.quadViewport(g);
	}
};

int main(){
	MyApp().start();
}
