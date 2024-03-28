/*
Allocore Example: Rendering text with Font

Description:
This example shows how to load fonts and render text at various locations
in the window.

Author:
Lance Putnam, Sept. 2013
*/

#include "allocore/io/al_App.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/graphics/al_Font.hpp"
using namespace al;

class MyApp : public App{
public:
	Font font1, font2, font3;

	MyApp(){
		std::string fontDir = "allocore/share/fonts/";
		if(!File::searchBack(fontDir)){
			printf("Error: Failed to find font directory\n");
			exit(-1);
		}

		// Args: font path, font size, anti-alias (default: true)
		bool good = true;
		good &= font1.load(fontDir + "Sansation.ttf", 20);
		good &= font2.load(fontDir + "VeraMoBd.ttf", 14);
		good &= font3.load(fontDir + "VeraMono.ttf", 10);
		if(!good){
			printf("Error: Failed to load font face\n");
			exit(-1);
		}

		initWindow(Window::Dim(400,200));
	}

	void onDraw(Graphics& g) override {

		// Get window dimensions, in pixels, for positioning the text
		float W = window().width();
		float H = window().height();

		// Setup our matrices for 2D pixel space
		g.pushMatrix(Graphics::PROJECTION);
		g.loadMatrix(Matrix4f::ortho2D(0, W, 0, H));
		g.pushMatrix(Graphics::MODELVIEW);

		// Before rendering text, we must turn on blending
		g.blendAdd();

		// Render text in the top-left corner
		g.loadIdentity();
		g.translate(8, H - 8);
		g.currentColor(1,1,0,1);
		font1.lineSpacing(1);
		font1.render(g, "Top-left text");

		// Render text in the bottom-left corner
		g.loadIdentity();
		g.translate(8, font3.size() + 8);
		g.currentColor(1,0,1,1);
		font3.render(g, "Bottom-left text");

		// Render text centered on the screen
		g.loadIdentity();
		std::string str =
			  " !\"#$%&'()*+,-./0123456789:;<=>?"
			"\n@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
			"\n`abcdefghijklmnopqrstuvwxyz{|}~ "
			"\nThe quick brown fox jumps over the lazy dog."
			"\nfor(int i=0; i<128; ++i){\n    printf(\"%d\\n\", i);\n}"
		;
		float bbw, bbh; font2.bounds(bbw,bbh, str);
		// Note that positions must integer to avoid blurred text
		g.translate(int(W/2 - bbw/2), int(H/2 + bbh/2));
		g.currentColor(0,1,1,1);
		font2.render(g, str);

		// Turn off blending
		g.blendOff();

		g.popMatrix();
		g.popMatrix(Graphics::PROJECTION);
	}
};


int main(){
	MyApp().start();
}
