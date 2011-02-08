#include "alloutil/al_World.hpp"
#include "allocore/graphics/al_Isosurface.hpp"


using namespace al;

class MyActor : public Actor{
public:

	MyActor(){}

	virtual void onDraw(Graphics& g, const Viewpoint& v){		
		Frustumd fr;
		v.camera().frustum(fr, v.transform(), v.viewport().aspect());

//		printf("ntl: %g %g %g\n", fr.ntl[0], fr.ntl[1], fr.ntl[2]);
//		printf("ftl: %g %g %g\n", fr.ftl[0], fr.ftl[1], fr.ftl[2]);

		Mesh& m = g.mesh();
		
		m.reset();
		m.primitive(g.LINES);
		m.vertex(-1,-1,11);
		m.vertex( 1, 1,12);

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

//		// draw frustum
//		m.reset();
//		m.vertex(fr.nbl);
//		m.vertex(fr.nbr);
//		m.vertex(fr.ntr);
//		m.vertex(fr.ntl);
//		m.vertex(fr.fbl);
//		m.vertex(fr.fbr);
//		m.vertex(fr.ftr);
//		m.vertex(fr.ftl);
//		m.color(Color(0.5,0.5,0.5));
//		m.color(Color(1,0,0));
//		m.color(Color(1,1,0));
//		m.color(Color(0,1,0));
//		m.color(Color(0,0,1));
//		m.color(Color(1,0,1));
//		m.color(Color(1,1,1));
//		m.color(Color(0,1,1));
//
//		{
//			int edges[] = {
//				0,1, 3,2, 7,6, 4,5,
//				5,2,
//				2,6, 1,5, 0,4, 3,7,
//			};
//			for(unsigned e=0; e<sizeof(edges)/sizeof(edges[0]); ++e){
//				m.index(edges[e]);
//			}
//		}
//		m.primitive(g.LINE_STRIP);
//		g.draw();
//
//		m.indices().reset();
//		{
//			int edges[] = {
//				3, 2, 0, 1, 7, 6, 4, 5
//			};
//			for(unsigned e=0; e<sizeof(edges)/sizeof(edges[0]); ++e){
//				m.index(edges[e]);
//			}
//		}
//
//		g.pointSize(20);
//		m.primitive(g.POINTS);		
//		g.draw();
	}

};


int main(){
	World w;
	w.name("Frustum Testing");
	w.camera().near(10).far(25);
	w.nav().quat().fromAxisAngle(0, 0,0,1);

	ViewpointWindow win(0,0, 600,400, w.name());
	Viewpoint vp;
	MyActor myActor;

	win.add(vp);
	w.add(win, true);
	w.add(myActor);

	w.start();
	return 0;
}

