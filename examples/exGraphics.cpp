
#include <stdio.h>
#include <stdlib.h>

#include "types/al_MsgQueue.hpp"
#include "system/al_MainLoop.hpp"
#include "io/al_WindowGL.hpp"
#include "protocol/al_Graphics.hpp"
#include "math/al_Random.hpp"
#include "system/al_Time.hpp"
#include "graphics/al_Context.hpp"
#include "spatial/al_Camera.hpp"
#include "protocol/al_GraphicsBackendOpenGL.hpp"

using namespace al;

Camera camera;
gfx::GraphicsBackendOpenGL backend;
gfx::Graphics gl(&backend);
gfx::Texture *texture = backend.textureNew();
gfx::Texture *surface_tex = backend.textureNew();
gfx::Surface *surface = backend.surfaceNew();
double eyesep = 1;

// synchronize window updates with a Clock. 
// every window has a clock, but can be optionally slaved to an external clock. 
// notify on clock should notify all windows, e.g. doFrmae()


class Clock;

class ClockListener {
public:
	virtual ~ClockListener() {}
	virtual void clockNotify(al_sec t, Clock * clock) = 0;
	
	Clock * clock() const { return mClock; }
	void setClock(Clock * c);
	
protected:
	friend class Clock;
	Clock * mClock;
};

class Clock {
public:
	Clock(al_sec period) : mDLL(period, 0.5), mIsRunning(false), mPeriod(period)
	{}
	
	Clock& period(al_sec p) { mPeriod = p; return *this; }
	al_sec period() const { return mPeriod; }
	bool isRunning() const { return mIsRunning; }
	
	Clock& start() {
		mIsRunning = true;
		MainLoop::queue().send(MainLoop::now(), notify, this);
		return *this;
	}
	
	Clock& stop() {
		mIsRunning = false;
		return *this;
	}
	
	Clock& add(ClockListener * listener) {
		if (listener->mClock)
			listener->mClock->remove(listener);
		mListeners.push_back(listener);
		listener->mClock = this;
		return *this;
	}
	
	/// potentially dangerous to call this during iteration (i.e. during notify())
	Clock& remove(ClockListener * listener) {
		mListeners.remove(listener);
		listener->mClock = NULL;
		return *this;
	}
	
	static void notify(al_sec t, Clock * self) { self->step(t); }
	
	void step(al_sec t) {
		if (mIsRunning) {
			for (std::list<ClockListener *>::iterator it = mListeners.begin(); it != mListeners.end(); ) {
				ClockListener * listener = *it;
				// removing this way makes it safe to remove during iteration:
				if (listener->clock() != this) {
					it = mListeners.erase(it);
				} else {
					listener->clockNotify(t, this);
					it++;
				}
			}
			
			al_sec rt = MainLoop::realtime();
			al_sec deadline = t+mPeriod;
			mDLL.step(rt);		
							
			if (rt > deadline) {
				// can't keep up; slip with respect to logical time				
				MainLoop::queue().send(rt+mPeriod*0.5, notify, this);
			} else {
				MainLoop::queue().send(deadline, notify, this);
			}
		}
	}

protected:
	bool mIsRunning;
	al_sec mPeriod;
	std::list<ClockListener *> mListeners;
	DelayLockedLoop mDLL;
};

void ClockListener::setClock(Clock * c) {
	c->add(this);
}	




al_sec period = 1./300;
Clock klock(period);


// invent some kind of scene:
struct vertex {
	float x, y, z;
	float r, g, b;
};
#define NUM_VERTICES (256)
static vertex vertices[NUM_VERTICES];

void mprint(const Matrix4d &m) {
	printf("%.5f %.5f %.5f %.5f\n", m[0], m[4], m[8], m[12]);
	printf("%.5f %.5f %.5f %.5f\n", m[1], m[5], m[9], m[13]);
	printf("%.5f %.5f %.5f %.5f\n", m[2], m[6], m[10], m[14]);
	printf("%.5f %.5f %.5f %.5f\n", m[3], m[7], m[11], m[15]);
}

static void drawScene(void * self) {

	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	

	gl.matrixMode(gfx::PROJECTION);
	gl.pushMatrix();
	//	gl.loadMatrix(camera.projectionMatrix());
		
	gl.matrixMode(gfx::MODELVIEW);
	gl.pushMatrix();
	//	gl.loadMatrix(camera.modelViewMatrix());
	

	surface->enter();
	gl.begin(gfx::LINES);
		for (int i=0; i< NUM_VERTICES; i++) {
			vertex& v = vertices[i];
			gl.color(v.r, v.g, v.b, 0.5);
			gl.vertex(v.x, v.y, v.z);
		}
		
		gl.color(1, 0, 0.5, 1);
		gl.vertex(-1, 0, 0);
		
		gl.color(1, 0, 0.5, 1);
		gl.vertex(1, 0, 0);
	gl.end();
	surface->leave();
	
	gl.pushMatrix();
	gl.scale(2, 2, 2);
		surface_tex->bind(0);
		gl.begin(gfx::QUADS);
			gl.color(1, 1, 1, 1);
			gl.texcoord(0, 0);
			gl.vertex(-1, -1);
			
			gl.texcoord(1, 0);
			gl.vertex(1, -1);
			
			gl.texcoord(1, 1);
			gl.vertex(1, 1);
			
			gl.texcoord(0, 1);
			gl.vertex(-1, 1);
		gl.end();
		surface_tex->unbind(0);
	gl.popMatrix();

	
	gl.pushMatrix();
	gl.translate(1, 1, 0);
		texture->bind(0);
		gl.draw();
		/*
		gl.begin(gfx::QUADS);
			gl.color(1, 1, 1, 1);
			gl.texcoord(0, 0);
			gl.vertex(-1, -1);
			
			gl.texcoord(1, 0);
			gl.vertex(1, -1);
			
			gl.texcoord(1, 1);
			gl.vertex(1, 1);
			
			gl.texcoord(0, 1);
			gl.vertex(-1, 1);
		gl.end();
		*/
		texture->unbind(0);
	gl.popMatrix();
	
	gl.matrixMode(gfx::PROJECTION);
	gl.popMatrix();
		
	gl.matrixMode(gfx::MODELVIEW);
	gl.popMatrix();
}



struct MyWindow : WindowGL{

	void onCreate(){ 					printf("onCreate\n"); }
	void onDestroy(){					printf("onDestroy\n"); }
	void onResize(int w, int h){		printf("onResize     %d, %d\n", w, h); }
	void onVisibility(bool v){			printf("onVisibility %s\n", v?"true":"false"); }
	
	void onKeyDown(const Keyboard& k){	printf("onKeyDown    "); printKey(); 
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
			case 'w': if(k.ctrl()) destroy(); break;
			case 'h': if(k.ctrl()) hide(); break;
			case 'm': if(k.ctrl()) iconify(); break;
			case 'c': if(k.ctrl()) cursorHideToggle(); break;
		}
	}
	void onKeyUp(const Keyboard& k){	printf("onKeyUp      "); printKey(); }
	
	void onMouseDown(const Mouse& m){	printf("onMouseDown  "); printMouse(); }
	void onMouseUp(const Mouse& m){		printf("onMouseUp    "); printMouse(); }
	void onMouseDrag(const Mouse& m){	printf("onMouseDrag  "); printMouse(); }
	//void onMouseMove(const Mouse& m){	printf("onMouseMove  "); printMouse(); }
	
	void printMouse(){
		const Mouse& m = mouse();
		printf("x:%4d y:%4d b:%d,%d\n", m.x(), m.y(), m.button(), m.down());
	}

	void printKey(){
		const Keyboard& k = keyboard();
		printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
	}

	void onFrame(){
		//drawScene(this);
		ctx.draw(drawScene, dimensions().w, dimensions().h, this);
	}
	
	Context ctx;
};

MyWindow win1;



class FrameClock : public ClockListener {

	virtual void clockNotify(al_sec t, Clock * clock) {
		
		camera.vec()[2] = sin(t*6)*2-12;
		camera.step();
			
		win1.doFrame();
		
//		al_sec rt = al_time() - MainLoop::T0();		// actual time now
//		al_sec next_t = t+clock->period();	
//		al_sec overtime = rt - next_t;						
	//	al_sec cpu = (overtime/clock->period())+1.;
		//printf("step/frame cpu: %6.2f%%, main cpu: %6.2f%%\n", cpu*100, MainLoop::cpu()*100);
	}
};

FrameClock frameclock;

double fps = 0;

int main (int argc, char * argv[]) {

	al::Lattice tex_data; 
	tex_data.create2d(3, AlloUInt8Ty, 256, 256);
	
	for(int j=0; j < 256; j++) {
		unsigned char *data = (unsigned char *)(tex_data.data.ptr + j*tex_data.header.stride[1]);
		
		for(int i=0; i < 256; i++) {
			*data++ = i;
			*data++ = j;
			*data++ = 255-j;
		}
	}
	
	texture->fromLattice(&tex_data);

	surface_tex->dimensions(512, 512);
	surface->attach(surface_tex);


	rnd::Random<> rng;

	/// define the scene - a semi random walk
	for(int i=0; i<NUM_VERTICES; i++) {
	
		double r, g, b, y, z;
		
		double p = i/(double)NUM_VERTICES;
		r = p;
		g = 1-p;
		b = sin(p * 6 * M_PI);
	
		y = b;
		z = 5 + 2*sin(p * 2 * M_PI);
		
		vertices[i].x = -1.5;
		vertices[i].y = y;
		vertices[i].z = z;
		vertices[i].r = r;
		vertices[i].g = g;
		vertices[i].b = b;
		
		i++;
		
		vertices[i].x = 1.5;
		vertices[i].y = y;
		vertices[i].z = z;
		vertices[i].r = r;
		vertices[i].g = g;
		vertices[i].b = b;

	} 
	
	// set fps == 0, and drive frame rendering manually instead
	win1.create(WindowGL::Dim(320, 240, 0, 40), "win1", fps);
	win1.ctx.viewport().camera(&camera);

//	camera.turn(Quatd::fromEuler(0., 0., 0.1));
	MainLoop::interval(0.01);
	
	klock.add(&frameclock);
	klock.start();
	
	
	MainLoop::start();
	return 0;
}
