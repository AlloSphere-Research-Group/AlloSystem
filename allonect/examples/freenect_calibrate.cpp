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

const char * man =
			"Freenect transform calibrator\n"
			"The goal is to align the area of interest as closely as possible with the virtual box. Walls, floors etc. should be just outside the virtual box, so that no unwanted noise enters the simulation.\n"
			"	1. move the point cloud into the center by dragging in the three plane views.\n"
			"	2. rotate the point cloud by dragging in the rotating orthographic view.\n"
			"	3. scale the point cloud by ctrl-dragging in any view.\n"
			"	4. make these adjustments more finely by holding down the shift key before dragging.\n"
			"	5. if it gets lost, press 'r' or 'z' to reset to the initial state.\n"
			"	6. To view in more detail and verify, you can press 'esc' to toggle full-screen. Press 'h' to toggle hide/show the particles outside the virtual box. Click on the video / depth image panes to switch the point cloud rendering source. \n"
			"	7. When you are happy, press 's' to print the transformation matrix to a file (it will be saved next to the application). Press 'p' to see this in the console.  Finally, press 'q' to quit. (If it crashes when quitting, don't worry.)\n"
;


struct MyWindow : public Window, public Freenect::Callback {

	MyWindow(int id, int dim = 1)
	:	Freenect::Callback(id),
		depthTex(640, 480),
		scale(dim / 4),
		azimuth(0),
		elevation(0),
		world_dim(dim),
		bUseVideo(0),
		bHideOOB(0),
		bReset(0)
	{
		pointsMesh.vertices().resize(640*480);
		pointsMesh.colors().resize(640*480);
		pointsMesh.texCoord2s().resize(640*480);

		translate.set(world_dim/2, world_dim/2, 0);
		updateTransform();

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

		startVideo();
		startDepth();
	}
	virtual ~MyWindow() {
		Freenect::stop();
	}

	void updateTransform() {
		transform =
			Matrix4f::translate(translate) *
			Matrix4f::rotate(-azimuth, Vec3f(0, 1, 0)) *
			Matrix4f::rotate(-elevation, Vec3f(1, 0, 0)) *
			Matrix4f::scale(scale, -scale, -scale);
	}

	virtual void onVideo(Texture& tex, uint32_t timestamp) {}

	virtual void onDepth(Texture& tex, uint32_t timestamp) {
		// get tilt:
		updateTransform();

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
					mmmx(veye.x);
					mmmy(veye.y);
					mmmz(veye.z);

					if (!bHideOOB || !OOB) {
						float a = 1. - OOB*0.7;
						colors[pts].set(a, a, a);
						vertices[pts].set(vworld.x, vworld.y, vworld.z);
						texCoord2s[pts].set(x*ix, y*iy);
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

	bool onCreate(){
		return true;
	}

	virtual bool onKeyDown(const Keyboard& k) {
		switch (k.key()) {
			case 'z':	// reset transform:
			case 'r':
				bReset = 1;
				break;
			case 's': {
					File f(paths.appPath() + "freenect_calibrate.lua", "w+", true);
					f.write("translate = ");
					translate.print(f.filePointer());
					fprintf(f.filePointer(), "\nscale = %f\n", scale);
					fprintf(f.filePointer(), "azimuth = %f\n", azimuth);
					fprintf(f.filePointer(), "elevation = %f\n", elevation);
					f.write("matrix = ");
					transform.print(f.filePointer());

				}
			case 'p':
				transform.print(stdout);
				printf("\n");
				break;
			case 'h':
				bHideOOB = !bHideOOB;
				break;
			case 'q':
				exit(0);
				break;
			default:
				break;
		}
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
		if (quadrant == 2) bUseVideo = 1;
		if (quadrant == 5) bUseVideo = 0;
		//printf("quad %d %d quadrant %d\n", quadx, quady, quadrant);

		return true;
	}

	virtual bool onMouseDrag(const Mouse& m) {
		float rate = 1. / panel_width;

		if (!keyboard().shift()) {
			rate *= world_dim;
		}

		printf("drag %f\n", rate);

		if (keyboard().ctrl()) {
			switch (quadrant) {
				case 1:
					elevation += (m.dx()) * rate / M_PI;
					break;
				default:
					// scale
					scale += (m.dx()) * rate;
			}
		} else {
			// translate
			switch (quadrant) {
				case 0:
					translate.x += m.dx() * rate;
					translate.z += m.dy() * rate;
					break;
				case 1:
					azimuth += (m.dx()) * rate / M_PI;
					break;
				case 3:
					translate.x += m.dx() * rate;
					translate.y -= m.dy() * rate;
					break;
				case 4:
					translate.z += m.dx() * rate;
					translate.y -= m.dy() * rate;
					break;
				default:
					break;
			}
		}

		return true;
	}

	bool onFrame(){
		float h = height();
		float w = width();

		if (bReset) {
			float wd2 = world_dim/2;
			elevation = tilt();
			azimuth = 0;
			translate.set(wd2/2, wd2/2, world_dim*1.5);
			scale = wd2;
			updateTransform();
			bReset = 0;
		}

		panel_width = h/2;

		float hdim = world_dim;

		gl.viewport(0, 0, w, h);
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

		gl.viewport(panel_width*2, 0, w - panel_width*2, h);
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::identity());
		gl.color(1, 1, 1, 1);
		video.quad	 (gl, 1, 0.5, 0, 0);
		depthTex.quad(gl, 1, 0.5, 0, 0.5);

		Vec3d center(world_dim/2, world_dim/2, world_dim/2);
		const double cbrt_1_3 = 0.577350269189626;

		// quadrant 4 (front)
		// looking down -z:	(x, y, -z)
		gl.viewport(0, 0, panel_width, panel_width);
		gl.projection(Matrix4d::ortho(
			-hdim, hdim,
			-hdim, hdim,
			-hdim, hdim
		));
		gl.modelView(Matrix4d::lookAt(
			center + Vec3d(0, 0, world_dim/2),
			center,
			Vec3d(0, 1, 0)
		));
		drawpoints();

		// quadrant 5 (side)
		// looking down x (z, y, x)
		gl.viewport(panel_width, 0, panel_width, panel_width);
		gl.projection(Matrix4d::ortho(
			-hdim, hdim,
			-hdim, hdim,
			-hdim, hdim
		));
		gl.modelView(Matrix4d::lookAt(
			center + Vec3d(-world_dim/2, 0, 0),
			center,
			Vec3d(0, 1, 0)
		));
		drawpoints();

		// quadrant 0 (top)
		// looking down y (x, -z, -y)
		gl.viewport(0, panel_width, panel_width, panel_width);
		gl.projection(Matrix4d::ortho(
			-hdim, hdim,
			-hdim, hdim,
			-hdim, hdim
		));
		gl.modelView(Matrix4d::lookAt(
			center + Vec3d(0, world_dim/2, 0),
			center,
			Vec3d(0, 0, -1)
		));
		drawpoints();

		// rotated:
		float t = MainLoop::now();
		float wd2 = world_dim/2;
		gl.viewport(panel_width, panel_width, panel_width, panel_width);
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
		if (bUseVideo) {
			video.bind();
		} else {
			depthTex.bind();
		}
		gl.draw(num_points, pointsMesh);
		if (bUseVideo) {
			video.unbind();
		} else {
			depthTex.unbind();
		}
		gl.draw(frameMesh);
	}

	Texture depthTex;
	Mesh pointsMesh;
	Mesh frameMesh;
	int num_points;
	MinMeanMax<> mmmx, mmmy, mmmz;
	Vec3f vmin, vmean, vmax, vrange;
	Matrix4f transform;
	Vec3f translate;
	float scale, azimuth, elevation;

	double world_dim;
	double panel_width;
	int quadrant, mx, my;

	bool bUseVideo, bHideOOB, bReset;
};

int main(int argc, char * argv[]){
	paths.addAppPaths(argc, argv);

	for (int id=0; id<2; id++) {

		MyWindow& win = *(new MyWindow(id));
		Lua L;
		L.dofile(paths.appPath() + "freenect_calibrate.lua");

		lua_getglobal(L, "matrix");
		int tab = lua_gettop(L);
		if (lua_istable(L, -1)) {
			printf("loading from freenect_calibrate.lua\n");
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
			lua_settop(L, 0);
			lua_getglobal(L, "scale");
			if (lua_isnumber(L, -1)) win.scale = lua_tonumber(L, -1);
			lua_settop(L, 0);
			lua_getglobal(L, "azimuth");
			if (lua_isnumber(L, -1)) win.azimuth = lua_tonumber(L, -1);
			lua_settop(L, 0);
			lua_getglobal(L, "elevation");
			if (lua_isnumber(L, -1)) win.elevation = lua_tonumber(L, -1);
			lua_settop(L, 0);
			lua_getglobal(L, "translate");
			if (lua_istable(L, -1)) {
				for (int i=1; i<=3; i++) lua_rawgeti(L, tab, i);
				win.translate.set(
					lua_tonumber(L, tab+1),
					lua_tonumber(L, tab+2),
					lua_tonumber(L, tab+3)
				);
			}
		} else {
			win.bReset = 1;
		}


		win.append(*new StandardWindowKeyControls);
		win.create(Window::Dim(800, 480));
	}

	printf(man);

	MainLoop::start();
	return 0;
}
