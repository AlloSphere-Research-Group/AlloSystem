#ifndef INCLUDE_AJIT_H
#define INCLUDE_AJIT_H

#include "allocore/graphics/al_Stereographic.hpp"
#include "allocore/io/al_Window.hpp"
#include "allocore/math/al_Random.hpp"

#include "al_Compiler.hpp"

#include "al_NavControl.hpp"
#include "al_Zone.hpp"

typedef void (*draw_fptr)(void *);

namespace al {

static rnd::Random<> rng;

class World;

template<typename T>
class BasicList {
public:
	typedef typename std::list<T *> List;
	typedef typename List::iterator Iter;
	
	void attach(T * o) { mObjects.push_back(o); }
	void detach(T * o) { mObjects.remove(o); }
	
	Iter begin() { return mObjects.begin(); }
	Iter end() { return mObjects.end(); }
	
	List mObjects;
};

class Drawables : public BasicList<Drawable> {
public:
	void onDraw(Graphics& gl) {
		Iter it = mObjects.begin();
		while (it != mObjects.end()) {
			Drawable& o = *(*it++);
			o.onDraw(gl);
		}
	}
};

/*
	Container for all AOT installed elements
*/
class World : public Drawable {
public:
	/// get the world singleton
	static World * get();
	
	struct InputControl : public NavInputControl {
		InputControl(World * W) : NavInputControl(&W->nav) {}
		bool onKeyDown(const Keyboard& k);
		bool onKeyUp(const Keyboard& k);		
		bool onMouseDrag(const Mouse& m){ return true; }
	};

	struct WorldWindow : public Window{
		WorldWindow() {}
		bool onFrame();
		static void doFrameNothing(void *) {};
	};
		
	virtual void onDraw(Graphics& gl) {
		drawables.onDraw(gl);
	};
	
	WorldWindow win;
	Graphics gl;
	Stereographic stereo;
	Camera cam;
	Nav nav;
	Drawables drawables;
	
private:
	World();
};






} // al::


#endif /* include guard */
