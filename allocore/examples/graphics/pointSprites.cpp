/*
Allocore Example: Point Sprites

Description:
This demonstrates OpenGL's built-in point sprite capability. A point sprite
is simply a screen-aligned textured square.

Author:
Lance Putnam, Feb. 2011
*/

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Texture.hpp"
using namespace al;

class MyApp : public App{
public:

	float angle = 0.;
	Mesh data;
	Texture tex{16,16, Graphics::LUMINANCE};

	MyApp(){
		// Generate a grid of points
		const int N = 12;
		data.points();

		for(int k=0; k<N; ++k){ float z = float(k)/(N-1)*2-1;
		for(int j=0; j<N; ++j){ float y = float(j)/(N-1)*2-1;
		for(int i=0; i<N; ++i){ float x = float(i)/(N-1)*2-1;
			data.vertex(x,y,z);
			data.color(x*0.1+0.1, y*0.1+0.1, z*0.1+0.1, 1);
		}}}

		// Create a Gaussian "bump" function to use for the sprite
		tex.assignFromTexCoord<Color>([](float u, float v){
			auto xy = toVec(u,v)*2.f-1.f;
			float m = exp(-3.*(xy.magSqr()));
			return Color(m);
		});

		nav().pullBack(6);
		initWindow();
	}

	void onAnimate(double dt) override {
		angle += 0.1;
		if(angle > 360.) angle -= 360.;
	}

	void onDraw(Graphics& g) override {

		// Tell GPU to render a screen-aligned textured quad at each vertex
		glEnable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

		// Setting the point size sets the sprite size
		g.pointSize(40);

		// Enable blending to hide texture edges
		g.blendAdd();

		// We must bind our sprite texture before drawing the points
		tex.bind();
			g.rotate(angle*7, 0,1,0);
			g.rotate(angle*3, 0,0,1);
			g.draw(data);
		tex.unbind();

		glDisable(GL_POINT_SPRITE);
	}
};

int main(){
	MyApp().start();
}
