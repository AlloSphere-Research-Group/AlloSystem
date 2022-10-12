/*
Allocore Example: Texture

Description:
This demonstrates how to create and display a texture.

Author:
Lance Putnam, Nov. 2013
*/

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/math/al_Functions.hpp" // clip, wrap
using namespace al;

class MyApp : public App{
public:

	// Construct texture
	// Arguments: width, height, pixel format, pixel data type
	// Note that when we construct a texture in this way, the default
	// policy is to allocate a client-side buffer to hold the pixels.
	Texture tex{64,64, Graphics::RGBA, Graphics::UBYTE};
	Mesh mesh;

	MyApp(){
		// The default magnification filter is linear
		//tex.filterMag(Texture::NEAREST);

		// Get a pointer to the (client-side) pixel buffer.
		// When we make a read access to the pixels, they are flagged as dirty
		// and get sent to the GPU the next time the texture is bound.
		auto * pixels = tex.data<unsigned char>();

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
			pixels[idx*stride + 0] = col.r * 255.;
			pixels[idx*stride + 1] = col.g * 255.;
			pixels[idx*stride + 2] = col.b * 255.;
			pixels[idx*stride + 3] = col.a;
		}}

		// Generate the geometry onto which to display the texture
		mesh.triangleStrip();
		mesh.vertex(-1,  1);
		mesh.vertex(-1, -1);
		mesh.vertex( 1,  1);
		mesh.vertex( 1, -1);

		// Add texture coordinates
		mesh.texCoord(0,1);
		mesh.texCoord(0,0);
		mesh.texCoord(1,1);
		mesh.texCoord(1,0);

		// The color acts as a multiplier on the texture color
		mesh.color(RGB(1)); // white: shows texture as is
		//mesh.color(RGB(1,0,0)); // red: shows only red components

		nav().pullBack(4);
		initWindow();
	}

	void onDraw(Graphics& g) override {
		// We must tell the GPU to use the texture when rendering primitives
		tex.bind();
			g.draw(mesh);
		tex.unbind();
	}
};

int main(){
	MyApp().start();
}
