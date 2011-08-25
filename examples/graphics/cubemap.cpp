#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlNav.hpp"
#include "alloutil/al_OpenGL.hpp"
//#include "alloutil/al_FrameBufferGL.hpp"

using namespace al;

static GraphicsGL gl;
static Mesh mesh, grid;
static Stereographic stereo;
static Camera cam;
Nav nav;

void viewOrtho(double w, double h)                            // Set Up An Ortho View
{
    glMatrixMode(GL_PROJECTION);                    // Select Projection
    glPushMatrix();                         // Push The Matrix
    glLoadIdentity();                       // Reset The Matrix
    glOrtho( 0, w , h , 0, -1, 1 );             // Select Ortho Mode (640x480)
    glMatrixMode(GL_MODELVIEW);                 // Select Modelview Matrix
    glPushMatrix();                         // Push The Matrix
    glLoadIdentity();                       // Reset The Matrix
}

void drawQuad() {
	gl.texCoord	( sin(-0.5*M_PI), sin(0.5*M_PI), 1);
	gl.vertex	( 0, 0, 0);
	gl.texCoord	( sin(-0.5*M_PI), sin(-0.5*M_PI), 1);
	gl.vertex	( 0, 1, 0);
	gl.texCoord	( sin(0.5*M_PI), sin(-0.5*M_PI), 1);
	gl.vertex	( 1, 1, 0);
	gl.texCoord	( sin(0.5*M_PI), sin(0.5*M_PI), 1);
	gl.vertex	( 1, 0, 0);
}

void drawVertex(double x, double y) {

	Vec3d v(1.-x*2., 1.-y*2., 1.);
	v.normalize();

	gl.texCoord	( v );
	gl.vertex	( x, y, 0);
}

void drawVertex1(double x, double y) {

	// x runs 0..1, convert to angle -PI..PI:
	double az = M_PI * (x*2.-1.);
	// y runs 0..1, convert to angle -PI_2..PI_2:
	double el = M_PI * 0.5 * (y*2.-1.);
	
	// convert polar to normal:
	double x1 = sin(az);
	double y1 = sin(el);
	double z1 = cos(az);

	Vec3d v(x1, y1, z1);
	v.normalize();

	gl.texCoord	( v );
	gl.vertex	( x, y, 0);
}

void drawQuad1() {
	double step = 1./30.;
	for (double x=0; x<=1.; x+=step) {
		for (double y=0; y<=1.; y+=step) {
			drawVertex1(x, y);
			drawVertex1(x, y+step);
			drawVertex1(x+step, y+step);
			drawVertex1(x+step, y);
		}
	}
}

struct MyWindow : Window, public Drawable{
	
	bool onKeyDown(const Keyboard& k){	
		
		switch(k.key()){
			case 'o': stereo.omni(!stereo.omni()); return false;
			case Key::Tab: stereo.stereo(!stereo.stereo()); return false;
			case '1': stereo.mode(Stereographic::Anaglyph); return false;
			case '2': stereo.mode(Stereographic::Active); return false;
			case '3': stereo.mode(Stereographic::Dual); return false;
			case '4': stereo.mode(Stereographic::LeftEye); return false;
			case '5': stereo.mode(Stereographic::RightEye); return false;
			default: return true;
		}
	}
	
	bool onCreate() {
		return true;
	}
	
	bool onDestroy() {
		return true;
	}

	bool onFrame(){
	
		int w = width();
		int h = height();
		
		int cubelen = 1024;
	
		unsigned int color_tex, fb, depth_rb;
		
		nav.step();
		
		
		Viewport vp(width(), height());
		Pose& pose = nav;
		double near = cam.near();
		double far = cam.far();
		const Vec3d& pos = pose.pos();
		double fovy = cam.fovy();
		double aspect = vp.aspect();
		Vec3d ux, uy, uz; pose.unitVectors(ux, uy, uz);
		Matrix4d mProjection = Matrix4d::perspective(90, 1, near, far);
		Matrix4d mModelViews[6];
		// GL_TEXTURE_CUBE_MAP_POSITIVE_X   
		mModelViews[0] = Matrix4d::lookAt(uz, uy, -ux, pos);
		// GL_TEXTURE_CUBE_MAP_NEGATIVE_X   
		mModelViews[1] = Matrix4d::lookAt(-uz, uy, ux, pos);
		// GL_TEXTURE_CUBE_MAP_POSITIVE_Y   
		mModelViews[2] = Matrix4d::lookAt(ux, -uz, uy, pos);
		// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y   
		mModelViews[3] = Matrix4d::lookAt(ux, uz, -uy, pos);
		// GL_TEXTURE_CUBE_MAP_POSITIVE_Z   
		mModelViews[4] = Matrix4d::lookAt(ux, uy, uz, pos);
		// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z   
		mModelViews[5] = Matrix4d::lookAt(-ux, uy, -uz, pos);
		
		stereo.clearColor(Color(0));
		
		//RGBA8 Cubemap texture, 24 bit depth texture, 256x256
		glGenTextures(1, &color_tex);
		glBindTexture(GL_TEXTURE_CUBE_MAP, color_tex);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//NULL means reserve texture memory, but texels are undefined
		for (int i=0; i<6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA8, cubelen, cubelen, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		}
		//-------------------------
		glGenFramebuffersEXT(1, &fb);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
		//Attach one of the faces of the Cubemap texture to this FBO
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, color_tex, 0);
		//-------------------------
		glGenRenderbuffersEXT(1, &depth_rb);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_rb);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, cubelen, cubelen);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_rb);
		//-------------------------
		//Does the GPU support current FBO configuration?
		GLenum status;
		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			printf("GPU does not support required FBO configuration\n");
			exit(0);
		}

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
		
		for (int i=0; i<6; i++) {
		
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, color_tex, 0);
		
			gl.viewport(0, 0, cubelen, cubelen);
			gl.clearColor(stereo.clearColor());
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			// apply camera transform:
			gl.pushMatrix(gl.PROJECTION);
			gl.loadMatrix(mProjection);
			gl.pushMatrix(gl.MODELVIEW);
			gl.loadMatrix(mModelViews[i]);
			
			onDraw(gl);
		
		}
		//Bind 0, which means render to back buffer, as a result, fb is unbound
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		
		// now draw texture:		
		gl.viewport(vp.l, vp.b, vp.w, vp.h);
		gl.clearColor(stereo.clearColor());
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		viewOrtho(1, 1);
		
		gl.lighting(false);
		gl.blending(false);
		
//		//tex.bind();
		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, color_tex);
		gl.color(1, 1, 1, 1);

		gl.begin(gl.QUADS);
		drawQuad1();
		gl.end();
		
		//tex.unbind();
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDisable(GL_TEXTURE_CUBE_MAP);
		
		//Delete resources
		glDeleteTextures(1, &color_tex);
		glDeleteRenderbuffersEXT(1, &depth_rb);
		glDeleteFramebuffersEXT(1, &fb);
		
		
		
/*		
		
		// draw 6 times to FBO, capture as texture
		fbo.onEnter();
		//stereo.draw(gl, cam, nav, vp, *this);
		
		Viewport vp(width(), height());
		Pose& pose = nav;
		double near = cam.near();
		double far = cam.far();
		const Vec3d& pos = pose.pos();
		
//		glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT);
//		gl.clearColor(stereo.clearColor());
		
		//glDrawBuffer(GL_BACK);
		gl.viewport(vp.l, vp.b, vp.w, vp.h);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		
		
		double fovy = cam.fovy();
		double aspect = vp.aspect();
		Vec3d ux, uy, uz; pose.unitVectors(ux, uy, uz);
		Matrix4d mProjection = Matrix4d::perspective(fovy, aspect, near, far);
		Matrix4d mModelView = Matrix4d::lookAt(ux, uy, uz, pos);

		// apply camera transform:
		gl.pushMatrix(gl.PROJECTION);
		gl.loadMatrix(mProjection);
		gl.pushMatrix(gl.MODELVIEW);
		gl.loadMatrix(mModelView);
		
		onDraw(gl);

		gl.popMatrix(gl.PROJECTION);
		gl.popMatrix(gl.MODELVIEW);
		
//		glPopAttrib();
		
		fbo.onLeave();
		
		// draw textures
		fbo.draw(gl);
*/		
		
		return true;
	}

	void onDraw(Graphics& gl){
		gl.fog(cam.far(), cam.far()/2, stereo.clearColor());
		gl.depthTesting(1);
		gl.draw(grid);
		gl.draw(mesh);	
	}
};


int main(){

	nav.smooth(0.8);
	nav.pos(0, 0, -20);

	cam.near(1).far(100).focalLength(1).fovy(45);
	cam.eyeSep(-cam.eyeSepAuto());
	//stereo.omni(true, 24, 120);
	stereo.stereo(false);
	stereo.mode(Stereographic::Anaglyph);
	
	// set up mesh:
	mesh.primitive(Graphics::TRIANGLES);
	double tri_size = 2;
	double world_radius = 50;
	int count = 4000;
	for (int i=0; i<count; i++) {
		double x = rnd::uniformS(world_radius);
		double y = rnd::uniformS(world_radius);
		double z = rnd::uniformS(world_radius);
		for (int v=0; v<3; v++) {
			mesh.color(HSV(float(i)/count, v!=2, 1));
			mesh.vertex(x+rnd::uniformS(tri_size), y+rnd::uniformS(tri_size), z+rnd::uniformS(tri_size));
		}
	}
	
	grid.primitive(Graphics::LINES);
	double stepsize = 1./2;
	for (double x=-1; x<=1; x+= stepsize) {
	for (double y=-1; y<=1; y+= stepsize) {
		grid.vertex(x, y, 1);
		grid.vertex(x, y, -1);
	}}
	for (double x=-1; x<=1; x+= stepsize) {
	for (double z=-1; z<=1; z+= stepsize) {
		grid.vertex(x, 1, z);
		grid.vertex(x, -1, z);
	}}
	for (double y=-1; y<=1; y+= stepsize) {
	for (double z=-1; z<=1; z+= stepsize) {
		grid.vertex(1, y, z);
		grid.vertex(-1, y, z);
	}}
	grid.scale(world_radius);
	
	MyWindow win;
	win.create(Window::Dim(100, 0, 640, 480), "Omnigraphic Example", 60);
	
	win.displayMode(win.displayMode() | DisplayMode::StereoBuf);

	win.add(new StandardWindowKeyControls);
	win.add(new NavInputControl(nav));

	MainLoop::start();
    return 0;
}
