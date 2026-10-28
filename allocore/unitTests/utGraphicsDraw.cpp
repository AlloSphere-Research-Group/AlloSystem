#include "utAllocore.h"
#include "allocore/io/al_Window.hpp"
#include "allocore/graphics/al_Graphics.hpp"

int utGraphicsDraw(){
	using namespace al;

	struct MyWindow : Window{

		Graphics g;

		bool onFrame(){

			g.clear(g.COLOR_BUFFER_BIT | g.DEPTH_BUFFER_BIT);
			g.viewport(0,0, rect().w, rect().h);
			g.loadIdentity();

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

			//gl.primitive(g.TRIANGLE_STRIP);
			data.primitive(g.LINE_STRIP);

			g.draw(data);

			return true;
		}

		Mesh data;
	};

	MyWindow win;

	win.create({400,400}, "Window", 40);

	Window::startLoop();
	return 0;
}
