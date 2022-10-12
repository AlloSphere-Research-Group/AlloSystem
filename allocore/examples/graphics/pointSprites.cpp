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

	float angle;
	Mesh data;
	Texture spriteTex;

	MyApp()
	:	spriteTex(16,16, Graphics::LUMINANCE, Graphics::FLOAT)
	{
		angle = 0;

		// Generate a grid of points
		const int N = 12;
		data.primitive(Graphics::POINTS);

		for(int k=0; k<N; ++k){ float z = float(k)/(N-1)*2-1;
		for(int j=0; j<N; ++j){ float y = float(j)/(N-1)*2-1;
		for(int i=0; i<N; ++i){ float x = float(i)/(N-1)*2-1;
			data.vertex(x,y,z);
			data.color(x*0.1+0.1, y*0.1+0.1, z*0.1+0.1, 1);
		}}}

		// Create a Gaussian "bump" function to use for the sprite
		int Nx = spriteTex.width();
		int Ny = spriteTex.height();
		float * pixels = spriteTex.data<float>();

		for(int j=0; j<Ny; ++j){ float y = float(j)/(Ny-1)*2-1;
		for(int i=0; i<Nx; ++i){ float x = float(i)/(Nx-1)*2-1;
			float m = exp(-3*(x*x + y*y));
			pixels[j*Nx + i] = m;
		}}

		nav().pullBack(6);
		initWindow();
	}

	void onAnimate(double dt){
		angle += 0.1;
		if(angle>360) angle -= 360;
	}

	void onDraw(Graphics& g){

		// Tell GPU to render a screen-aligned textured quad at each vertex
		glEnable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

		// Setting the point size sets the sprite size
		g.pointSize(40);

		// Enable blending to hide texture edges
		g.blendAdd();

		// We must bind our sprite texture before drawing the points
		spriteTex.bind();
			g.rotate(angle*7, 0,1,0);
			g.rotate(angle*3, 0,0,1);
			g.draw(data);
		spriteTex.unbind();

		glDisable(GL_POINT_SPRITE);
	}
};

int main(){
	MyApp().start();
}
