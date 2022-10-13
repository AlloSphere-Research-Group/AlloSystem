/*
Allocore Example: Frame Feedback

Description:
This demonstrates how to create a feedback post-processing effect.
This is accomplished by copying the frame buffer into a texture after rendering
and then displaying the texture at the start of the next frame. Different
feedback effects can be accomplished by distorting the quad the texture is
rendered onto.

Author:
Lance Putnam, Nov. 2014
*/
#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Texture.hpp"

using namespace al;

class MyApp : public App{
public:

	Mesh shape;
	Texture texBlur;
	float angle = 0.f;

	MyApp(){
		// Create a colored square
		shape.primitive(Graphics::LINE_LOOP);
		const int N = 4;
		for(int i=0; i<N; ++i){
			float theta = float(i)/N * 2*M_PI;
			shape.vertex(cos(theta), sin(theta));
			shape.color(HSV(theta/2/M_PI));
		}

		nav().pullBack(4);
		initWindow();
	}

	virtual void onAnimate(double dt) override {
		angle += dt * 90;
		if(angle >= 360) angle -= 360;
	}

	virtual void onDraw(Graphics& g) override {

		// 1. Match texture dimensions to viewport
		texBlur.resize(viewport().w, viewport().h);
	
		// 2. Draw feedback texture. Try the different varieties!
		// Note, we do not want to draw the texture if has just been resized.
		if(!texBlur.shapeUpdated()){
			// Plain (non-transformed) feedback
			//texBlur.quadViewport(g, RGB(0.98));

			// Outward feedback
			texBlur.quadViewport(g, RGB(0.98), 2.01, 2.01, -1.005, -1.005);

			// Inward feedback
			//texBlur.quadViewport(g, RGB(0.98), 1.99, 1.99, -0.995, -0.995);

			// Oblate feedback
			//texBlur.quadViewport(g, RGB(0.98), 2.01, 2.0, -1.005, -1.00);

			// Squeeze feedback!
			//texBlur.quadViewport(g, RGB(0.98), 2.01, 1.99, -1.005, -0.995);
		}

		// 3. Do your drawing...
		g.matrixScope([&](){
			g.rotate(angle, 0,0,1);
			g.draw(shape);
		});


		// 4. Copy current (read) frame buffer to texture
		texBlur.copyFrameBuffer();
	}
};

int main(){
	MyApp().start();
}
