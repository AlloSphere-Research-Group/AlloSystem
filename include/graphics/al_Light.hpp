#ifndef INCLUDE_AL_GRAPHICS_LIGHT_HPP
#define INCLUDE_AL_GRAPHICS_LIGHT_HPP

#include "types/al_Color.hpp"

namespace al {
namespace gfx{

class Light{
public:
	Light(float x=5, float y=2, float z=5);
	~Light();

	Light& operator()();

	Light& ambient(const Color& v);
	Light& diffuse(const Color& v);
	Light& specular(const Color& v);
	Light& pos(float x, float y, float z);

	template <class VEC3>
	Light& pos(const VEC3& v){ return pos(v[0], v[1], v[2]); }

	const Color& ambient() const { return mAmbient; }
	const Color& diffuse() const { return mDiffuse; }
	const Color& specular() const { return mSpecular; }

protected:
	int mID;
	Color mAmbient;
	Color mDiffuse;
	Color mSpecular;
	float mPos[4];
};

} // ::al::gfx
} // ::al

#endif
