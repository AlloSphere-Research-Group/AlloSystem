/*

Example use of libfreenect in AlloCore

Build with:
-lfreenect -lusb-1.0 -framework Carbon -framework IOKit

Author:
Graham Wakefield, 2011
*/

#include "allocore/al_Allocore.hpp"
#include "allonect/al_Freenect.hpp"

using namespace al;

Graphics gl;
SearchPaths paths;

struct MyWindow : public Window, public Freenect1::Callback {

	MyWindow(int dim = 32) 
	:	Freenect1::Callback(0), 
		depthTex(640, 480),
		scale(1), 
		azimuth(0),
		elevation(0),
		world_dim(dim), 
		bHideOOB(0)  
	{
		pointsMesh.vertices().size(640*480);
		pointsMesh.colors().size(640*480);
		pointsMesh.texCoord2s().size(640*480);
		
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
		Freenect1::stop();
	}
	
	// @see http://nicolas.burrus.name/index.php/Research/KinectCalibration
	Vec3f depthToEye(int x, int y, uint16_t d) {
		// size of a pixel in meters, at zero/near plane:
		static const double metersPerPixelX = 1./594.21434211923247;
		static const double metersPerPixelY = 1./591.04053696870778;
		// location of X,Y corner at zero plane, in pixels:
		static const double edgeX = 339.30780975300314;	// x edge pixel
		static const double edgeY = 242.73913761751615;	// y edge pixel
		const double meters = rawDepthToMeters(d);
		const double disparityX = (x - edgeX);	// in pixels
		const double disparityY = (y - edgeY);	// in pixels
		return Vec3f(meters * disparityX * metersPerPixelX,
					 meters * disparityY * metersPerPixelY,
					 meters
					);
	}
	
	void updateTransform() {
		transform = 
			Matrix4f::translate(translate) * 
			Matrix4f::rotate(-azimuth, Vec3f(0, 1, 0)) * 
			Matrix4f::rotate(-elevation, Vec3f(1, 0, 0)) * 
			Matrix4f::scale(scale, -scale, -scale);
	}
	
	virtual void onVideo(Texture& tex, uint32_t timestamp) {
		
	}
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
		
		
		depthTex.updatePixels();
	}

	bool onCreate(){
		return true;
	}
	
	virtual bool onKeyDown(const Keyboard& k) {
		Vec3f def(world_dim/2, world_dim/2, 0);
		switch (k.key()) {
			case 'z':	// reset transform:
				elevation = tilt();
				translate.set(def);
				break;
			case 'r':	// reset transform:
				elevation = tilt();
				translate += def-vmean;
				break;
			case 's': {
					File f(paths.appPath() + "calibration.lua", "w", true);
					f.write("freenect_calibrate = ");
					transform.print(f.filePointer());
				}
			case 'p':
				transform.print(stdout);
				printf("\n");
				break;
			case 'h':
				bHideOOB = !bHideOOB;
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
		//printf("quad %d %d quadrant %d\n", quadx, quady, quadrant);
		
		return true;
	}
	
	virtual bool onMouseDrag(const Mouse& m) {
		if (keyboard().ctrl()) {
			switch (quadrant) {
				case 0:
				case 3:
				case 4:
					// scale
					scale += (m.dx()) / panel_width;
					printf("scale %f\n", scale);
					break;
				case 1:
					elevation += m.dy() / panel_width;
					printf("elevation %f (tilt %f)\n", elevation, tilt());
					break;
				default:
					break;
			}
		} else {
			// translate
			switch (quadrant) {
				case 0:
					translate.x += m.dx() / panel_width;
					translate.z += m.dy() / panel_width;
					break;
				case 1: 
					azimuth += m.dx() / panel_width;
					printf("azimuth %f\n", azimuth);
					break;
				case 3:
					translate.x += m.dx() / panel_width;
					translate.y -= m.dy() / panel_width;
					break;
				case 4:
					translate.z += m.dx() / panel_width;
					translate.y -= m.dy() / panel_width;
					break;
				default:
					break;
			}
			printf("translate "); translate.print(); printf("\n");
		}
		
		return true;
	}
	
	bool onFrame(){
		float h = height();
		float w = width();
		
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
	Vec3f translate;
	float scale, azimuth, elevation;
	
	double world_dim;
	double panel_width;
	int quadrant, mx, my;
	
	bool bHideOOB;
};

MyWindow win(4);

int main(int argc, char * argv[]){

	paths.addAppPaths(argc, argv);

	win.append(*new StandardWindowKeyControls);
	win.create(Window::Dim(800, 480));
	
	MainLoop::start();
	return 0;
}