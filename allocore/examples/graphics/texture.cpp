/*
Allocore Example: Texture

Description:
This demonstrates how to create and display a texture.

Author:
Lance Putnam, Nov. 2013
*/

#include "allocore/io/al_App.hpp"
using namespace al;

struct MyApp : App{

	Texture tex;

	MyApp()
		// arguments: width, height, pixel format, pixel data type
	:	tex(63,63, Graphics::RGB, Graphics::UBYTE)
	{
		// The default magnification filter is linear
		//tex.filterMag(Texture::NEAREST);

		// Allocate memory for the pixels
		tex.allocate();

		// Get a pointer to the pixel buffer
		unsigned char * texBuf = tex.data<unsigned char>();

		// Loop through the pixels to generate an image
		int Nx = tex.width();
		int Ny = tex.height();
		for(int j=0; j<Ny; ++j){ float y = float(j)/(Ny-1)*2-1;
		for(int i=0; i<Nx; ++i){ float x = float(i)/(Nx-1)*2-1;

			float m = 1 - al::clip(hypot(x,y));
			float a = al::wrap(atan2(y,x)/M_2PI);

			Color col = HSV(a,1,m);

			int idx = j*Nx + i;
			int stride = tex.numComponents();
			texBuf[idx*stride + 0] = col.r * 255.;
			texBuf[idx*stride + 1] = col.g * 255.;
			texBuf[idx*stride + 2] = col.b * 255.;
			//texBuf[idx*4 + 3] = col.a;
		}}

		// We must indicate when the pixels have been updated to ensure they get
		// (re)submitted to the GPU.
		tex.dirty();

		nav().pos().set(0,0,4);
		initWindow();
	}

	void onDraw(Graphics& g, const Viewpoint& vp){

		// Borrow a temporary Mesh from Graphics
		Mesh& m = g.mesh();

		m.reset();

		// Generate geometry
		m.primitive(Graphics::TRIANGLE_STRIP);
		m.vertex(-1,  1);
		m.vertex(-1, -1);
		m.vertex( 1,  1);
		m.vertex( 1, -1);

		// Add texture coordinates
		m.texCoord(0,1);
		m.texCoord(0,0);
		m.texCoord(1,1);
		m.texCoord(1,0);

		tex.bind();
			g.draw(m);
		tex.unbind();
	}
};

int main(){
	MyApp().start();
}
