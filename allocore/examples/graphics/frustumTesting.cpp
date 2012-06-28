#include "allocore/io/al_App.hpp"

using namespace al;

class MyApp : public App{
public:

	virtual void onDraw(Graphics& g, const Viewpoint& v){		
		Frustumd fr;
		v.lens().frustum(fr, v.worldTransform(), v.viewport().aspect());

//		printf("ntl: %g %g %g\n", fr.ntl[0], fr.ntl[1], fr.ntl[2]);
//		printf("ftl: %g %g %g\n", fr.ftl[0], fr.ftl[1], fr.ftl[2]);

		Mesh& m = g.mesh();
		
		m.reset();
		m.primitive(g.LINES);
		m.vertex(-1,-1, -11);
		m.vertex( 1, 1, -12);

		for(int i=0; i<m.vertices().size(); ++i){
			int r = fr.testPoint(m.vertices()[i]);
			
			m.color(HSV(r ? 0.3 : 0));
		}

		g.lineWidth(10);
		g.antialiasing(g.NICEST);
		g.draw();

		{
//			int r = fr.testPoint(Vec3d(0,0,15));
//			printf("%d\n", r);
		}

		// draw rectangle across frustum diagonal
		m.reset();
		m.color(Color(0.5));
		m.vertex(fr.nbl);
		m.vertex(fr.fbr);
		m.vertex(fr.ntr);
		m.vertex(fr.ftl);
		m.primitive(g.LINE_LOOP);
		g.draw();
	}

};

MyApp app;

int main(){
	app.initWindow();
	app.lens().near(10).far(25);
	app.start();
	return 0;
}


