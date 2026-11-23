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

	{ // copy
		Mesh a;
		a	.triangles()
			.stroke(13)
			.attribHint(Mesh::NORMAL | Mesh::TANGENT)
			.visible(false)
			.dirty(true)
		;
		a.vertex(10,20,30);
		auto b = a;
		assert(a.vertices().size() == b.vertices().size());
		assert(a.vertices().data() != b.vertices().data());
		assert(a.primitive() == b.primitive());
		assert(a.stroke() == b.stroke());
		assert(a.attribHint() == b.attribHint());
		assert(a.visible() == b.visible());
		assert(a.dirty() == b.dirty());
		assert(a.vertices()[0] == b.vertices()[0]);
		a.vertices()[0] *= 2.f;
		assert(a.vertices()[0] != b.vertices()[0]);
		b.lines();
		b.vertex(7,8,9);
		a = b;
		assert(a.vertices().size() == b.vertices().size());
		assert(a.vertices().data() != b.vertices().data());
		assert(a.vertices()[0] == b.vertices()[0]);
		assert(a.vertices()[1] == b.vertices()[1]);
	}

	{ // move
		Mesh a;
		Vec3f pos(10,20,30);
		a.vertex(pos);

		const auto * vptr = a.vertices().data();

		auto b = std::move(a);

		assert(a.vertices().size() == 0);
		assert(a.vertices().data() != vptr);
		assert(b.vertices().size() == 1);
		assert(b.vertices()[0] == pos);
		assert(b.vertices().data() == vptr);
	}

	return 0;
}
