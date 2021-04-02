#include "utAllocore.h"
#include "allocore/graphics/al_Mesh.hpp"

int utGraphicsMesh(){
	using namespace al;

	{
		const int N = 3;
		Vec3f verts[N] = { Vec3f(1,0,0), Vec3f(0,1,0), Vec3f(0,0,1)};
		Color colors[N] ={ Color(1,0,0), Color(0,1,0), Color(0,0,1)};

		assert(Color(0) == Color(0)); // ensure we can compare colors

		{
			Mesh m;
			for(int i=0; i<N; ++i){
				m.color(colors[i]);
				m.vertex(verts[i]);
			}

			for(int i=0; i<N; ++i){
				assert(m.vertices()[i] == verts[i]);
				assert(m.colors()[i] == colors[i]);
			}
		}

		{
			Mesh m;
			for(int i=0; i<N; ++i){
				m.vertex(verts[i]);
				m.color(colors[i]);
			}

//			for(int i=0; i<N; ++i){
//				printf("truth:   "); al::print(&verts[i][0], 3, "; "); al::println(&colors[i][0], 4);
//				printf("in mesh: "); al::print(&m.vertices()[i][0], 3, "; "); al::println(&m.colors()[i][0], 4);
//				printf("\n");
//			}

			for(int i=0; i<N; ++i){
				assert(m.vertices()[i] == verts[i]);
				assert(m.colors()[i] == colors[i]);
			}
		}

	}

	return 0;
}
