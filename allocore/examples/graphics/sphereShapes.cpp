/*
Allocore Example: Sphere Shapes

Description:
This renders a grid of "UV" spheres at various resolutions. The spheres are
generated from stacks of rings in spherical coordinates.

Author(s):
Lance Putnam, April 2011
*/

#include "allocore/al_Allocore.hpp"
using namespace al;

class MyApp : public App {
public:

	Mesh shapes;
	Light light;
	double angle;
	bool wireframe;
	bool vertexLight;

    MyApp()
	:	angle(0), wireframe(false), vertexLight(false)
	{
		nav().pos(0,0,4.3);
		initWindow(Window::Dim(600,600), "Sphere Shapes");
	}

	void onAnimate(double dt){
		angle += 0.0007;
		if(angle > M_2PI) angle -= M_2PI;

		shapes.reset();
		const int N = 8;
		for(int slices=3; slices<3+N; ++slices){
		for(int stacks=2; stacks<2+N; ++stacks){

			float px = float(slices-3)/(N-1) * 2 - 1;
			float py = float(stacks-2)/(N-1) * 2 - 1;

			int Nv = addSphere(shapes,1./N, slices,stacks);

			Mat4f xfm;
			xfm.setIdentity();
			xfm.rotate(angle*13, 0,1).rotate(angle*17, 1,2);
			xfm.translate(Vec3f(px, py, 0));
			shapes.transform(xfm, shapes.vertices().size()-Nv);

			for(int i=0; i<Nv; ++i){
				float v = float(i)/Nv;
				shapes.color(HSV(0.2*v, 1-v*0.5, 1));
			}
		}}

		if(!vertexLight) shapes.decompress();
		shapes.generateNormals();
	}

	void onDraw(Graphics& g){
		g.polygonMode(wireframe ? Graphics::LINE : Graphics::FILL);
		g.enable(Graphics::COLOR_MATERIAL);
		light.pos(4,4,1)();
		g.draw(shapes);
	}

	void onKeyDown(const Keyboard& k){
		switch(k.key()){
		case 'f': wireframe  ^=true; break;
		case 'l': vertexLight^=true; break;
		}
	}
};

int main(){
    MyApp().start();
}
