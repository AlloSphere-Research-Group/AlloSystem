/*

Example use of libfreenect in AlloCore

Build with:
-lfreenect -lusb-1.0 -framework Carbon -framework IOKit

Author:
Graham Wakefield, 2011
*/

#include "allocore/al_Allocore.hpp"
#include "allonect/al_Freenect.hpp"
#include "alloutil/al_Lua.hpp"

using namespace al;

Graphics gl;
SearchPaths paths;

struct MyWindow : public Window, public Freenect::Callback, public ThreadFunction {

	MyWindow(int dim = 1)
	:	Freenect::Callback(0),
		depthTex(640, 480),
		world_dim(dim),
		bHideOOB(0)
	{
		pointsMesh.vertices().resize(640*480);
		pointsMesh.colors().resize(640*480);
		pointsMesh.texCoord2s().resize(640*480);

		frameMesh.reset();
		frameMesh.primitive(gl.LINES);
		float c = 0.2;
		for (int a1=0; a1<2; a1++) {
		for (int a2=0; a2<2; a2++) {
			frameMesh.color(c, c+a1, c+a2);
			frameMesh.vertex(0, a1, a2);
			frameMesh.color(1, c+a1, c+a2);
			frameMesh.vertex(1, a1, a2);

			frameMesh.color(c+a1, c, c+a2);
			frameMesh.vertex(a1, 0, a2);
			frameMesh.color(c+a1, 1, c+a2);
			frameMesh.vertex(a1, 1, a2);

			frameMesh.color(c+a1, c+a2, c);
			frameMesh.vertex(a1, a2, 0);
			frameMesh.color(c+a1, c+a2, 1);
			frameMesh.vertex(a1, a2, 1);
		}}
		frameMesh.scale(world_dim);

		if (netSend.open(4111, "localhost")) {
			active = true;
			sendThread.start(*this);
		} else {
			printf("error creating netsend\n");
			exit(0);
		}

		startVideo();
		startDepth();
	}
	virtual ~MyWindow() {
		sendThread.join();
		Freenect::stop();
	}

	virtual void onVideo(Texture& tex, uint32_t timestamp) {

	}
	virtual void onDepth(Texture& tex, uint32_t timestamp) {

		Array& outArray = depthTex.array();
		uint16_t * depthPtr = (uint16_t*)tex.data();

		size_t i=0;

		Buffer<Mesh::Vertex>& vertices = pointsMesh.vertices();
		Buffer<Color>& colors = pointsMesh.colors();
		Buffer<Mesh::TexCoord2>& texCoord2s = pointsMesh.texCoord2s();
		int pts = 0;

		const float ix = 1./640;
		const float iy = 1./480;

		mmmx.clear();
		mmmy.clear();
		mmmz.clear();

//		mmmx(0);
//		mmmy(0);
//		mmmz(0);
//		mmmx(world_dim);
//		mmmy(world_dim);
//		mmmz(world_dim);

		for (size_t y=0; y<480; y++) {
			for (size_t x=0; x<640; x++, i++) {
				uint16_t raw = depthPtr[i];

				// ignore out-of-range points:
				if (raw < 2047) {
					// drawing raw data:
					HSV hsv(raw/2048., 1, 0.75);
					Colori c(hsv);
					outArray.write(c.components, x, y);

					// this vector is in eye-space of kinect;
					// it must be rotated into the kinect pose.
					Vec3f veye = depthToEye(x, y, raw);

					//Vec4f vworld = veye*scale + translate;
					Vec4f vworld = transform.transform(veye);

					bool OOB = (vworld.x < 0 || vworld.x > world_dim ||
						vworld.y < 0 || vworld.y > world_dim ||
						vworld.z < 0 || vworld.z > world_dim);

					// analysis:
					mmmx(vworld.x);
					mmmy(vworld.y);
					mmmz(vworld.z);

					if (!bHideOOB || !OOB) {
						float a = 1. - OOB*0.7;
						vertices[pts].set(vworld.x, vworld.y, vworld.z);
						texCoord2s[pts].set(x*ix, y*iy);
						colors[pts].set(a, a, a);
						pts++;
					}
				}
			}
		}
		num_points = pts;
		vmin.set(mmmx.min(), mmmy.min(), mmmz.min());
		vmean.set(mmmx.mean(), mmmy.mean(), mmmz.mean());
		vmax.set(mmmx.max(), mmmy.max(), mmmz.max());
		vrange.set(mmmx.max()-mmmx.min(), mmmy.max()-mmmy.min(), mmmz.max()-mmmz.min());


		depthTex.dirty();
	}

	// ThreadFunction:
	virtual void operator()() {
		Buffer<Vec3f>& vertices = pointsMesh.vertices();
		while (active) {
			// now send (some of) these data points:
			int limit = num_points;
			int step = 1 + limit/1024;
			int offset = rng.uniform(step);
			int count = 0;
			for (int i=offset;i<limit; i+= step) {
				count++;
				// send pixel i
				Vec3f& p = vertices[i];
				netSend.send("/point", p.x, p.y, p.z);
				//printf("/point %f %f %f\n", p.x, p.y, p.z);
			}
			al_sleep(0.01);
		}
	}

	bool onCreate(){
		return true;
	}

	virtual bool onKeyUp(const Keyboard& k){

		return true;
	}

	virtual bool onMouseDown(const Mouse& m) {
		// which quadrant?
		mx = m.x();
		my = m.y();
		int quadx = m.x() / panel_width;
		int quady = m.y() / panel_width;
		quadrant = quadx + quady*3;
		//printf("quad %d %d quadrant %d\n", quadx, quady, quadrant);

		return true;
	}

	bool onFrame(){
		float h = height();
		float w = width();

		panel_width = h/2;

		float hdim = world_dim;
		Vec3d center(world_dim/2, world_dim/2, world_dim/2);
		const double cbrt_1_3 = 0.577350269189626;

		gl.viewport(0, 0, w, h);
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

		// rotated:
		float t = MainLoop::now() * 0.2;
		float wd2 = world_dim/2;
		gl.projection(Matrix4d::ortho(
			-hdim, hdim,
			-hdim, hdim,
			-hdim*2, hdim*2
		));
		gl.modelView(Matrix4d::lookAt(
			center + Vec3d(-wd2*cos(t), wd2, wd2*sin(t)),
			center,
			Vec3d(cbrt_1_3*cos(t), cbrt_1_3, -cbrt_1_3*sin(t))
		));
		drawpoints();

		return true;
	}

	void drawpoints() {
		gl.depthTesting(true);
		gl.color(1, 1, 1, 1);
		video.bind();
		gl.draw(num_points, pointsMesh);
		video.unbind();
		gl.draw(frameMesh);
	}

	Texture depthTex;
	Mesh pointsMesh;
	Mesh frameMesh;
	int num_points;
	MinMeanMax<> mmmx, mmmy, mmmz;
	Vec3f vmin, vmean, vmax, vrange;
	Matrix4f transform;

	double world_dim;
	double panel_width;
	int quadrant, mx, my;

	osc::Send netSend;
	Thread sendThread;
	rnd::Random<> rng;

	bool bHideOOB;
	bool active;
};

MyWindow win;

int main(int argc, char * argv[]){

	paths.addAppPaths(argc, argv);

	Lua L;
	L.dofile(paths.appPath() + "freenect_calibrate.lua");

	lua_getglobal(L, "freenect_calibrate");
	int tab = lua_gettop(L);
	for (int i=1; i<=16; i++) lua_rawgeti(L, tab, i);
	win.transform = Matrix4f(
		lua_tonumber(L, tab+1), lua_tonumber(L, tab+2),
		lua_tonumber(L, tab+3), lua_tonumber(L, tab+4),
		lua_tonumber(L, tab+5), lua_tonumber(L, tab+6),
		lua_tonumber(L, tab+7), lua_tonumber(L, tab+8),
		lua_tonumber(L, tab+9), lua_tonumber(L, tab+10),
		lua_tonumber(L, tab+11), lua_tonumber(L, tab+12),
		lua_tonumber(L, tab+13), lua_tonumber(L, tab+14),
		lua_tonumber(L, tab+15), lua_tonumber(L, tab+16)
	);

	win.append(*new StandardWindowKeyControls);
	win.create(Window::Dim(480, 480));

	MainLoop::start();
	return 0;
}
