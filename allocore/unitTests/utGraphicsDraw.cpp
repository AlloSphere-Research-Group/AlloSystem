#include "utAllocore.h"
#include "allocore/io/al_Window.hpp"
#include "allocore/graphics/al_Graphics.hpp"

int utGraphicsDraw(){
	using namespace al;

	struct MyWindow2 : Window{

		Graphics gl;

		bool onFrame(){

			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			gl.viewport(0,0, dimensions().w, dimensions().h);
			gl.loadIdentity();

			data.reset();

			// test vertices and colors
			int N=19;
			for(int i=0; i<=N; ++i){
				float p = float(i)/N;
				data.vertex(cos(p*M_2PI), sin(p*M_2PI));
				data.color(p, p, 1);
			}

			// test rendering from index array
			for(int i=0; i<=N; ++i){
				data.index((i*(N/2-1)) % N);
			}

			//gl.primitive(gl.TRIANGLE_STRIP);
			data.primitive(gl.LINE_STRIP);

			gl.draw(data);

			return true;
		}

		Mesh data;
	};

	MyWindow2 win;

	win.create(Window::Dim(400,400), "Window 1", 40);

	Window::startLoop();
	return 0;
}
