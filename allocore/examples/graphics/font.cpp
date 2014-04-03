/*
Allocore Example: Rendering text with Font

Description:
This example shows how to load fonts and render text at various locations
in the window.

Author:
Lance Putnam, 9/2013
*/

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Font.hpp"
using namespace al;

class MyApp : public App{
public:

	Font font1;
	Font font2;
	Font font3;

	MyApp()
	:	font1("allocore/share/fonts/VeraMoIt.ttf", 20),
		font2("allocore/share/fonts/VeraMoBd.ttf", 14),
		font3("allocore/share/fonts/VeraMono.ttf", 10)
	{
		nav().pos(0,0,4);
		initWindow(Window::Dim(400,200));
	}

	virtual void onAnimate(double dt){}
	
	virtual void onDraw(Graphics& g, const Viewpoint& v){

		// Before rendering text, we must turn on blending
		g.depthTesting(true);
		g.depthMask(false);
		g.blending(true);
		g.blendModeAdd();

		// Get the viewport dimensions, in pixels, for positioning the text
		float W = v.viewport().w;
		float H = v.viewport().h;

		// Next, we need to setup our matrices for 2D pixel space
		g.pushMatrix(Graphics::PROJECTION);
		g.loadMatrix(Matrix4f::ortho2D(0, W, 0, H));
		g.pushMatrix(Graphics::MODELVIEW);

		// Render text in the top-left corner
		g.loadIdentity();
		g.translate(8, H - (font1.size() + 8));
		g.currentColor(1,1,0,1);
		font1.render(g, "Top-left text");

		// Render text in the bottom-left corner
		g.loadIdentity();
		g.translate(8, 8);
		g.currentColor(1,0,1,1);
		font3.render(g, "Bottom-left text");

		// Render text centered on the screen
		g.loadIdentity();
		std::string str = "Centered text";
		// Note that dimensions must be integers to avoid blurred text
		g.translate(int(W/2 - font2.width(str)/2), int(H/2 - font2.size()/2));
		g.currentColor(0,1,1,1);
		font2.render(g, str);

		g.popMatrix();
		g.popMatrix(g.PROJECTION);
	}
};


int main(){
	MyApp().start();
}
