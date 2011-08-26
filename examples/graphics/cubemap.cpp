#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlNav.hpp"
#include "alloutil/al_OpenGL.hpp"
//#include "alloutil/al_FrameBufferGL.hpp"

#define TEST

using namespace al;

static GraphicsGL gl;
static Mesh mesh, grid;
static Stereographic stereo;
static Camera cam;
Nav nav;

class CubeMapTexture : public GPUObject {
public:
	CubeMapTexture(int resolution=1024) 
	:	GPUObject(),
		mResolution(resolution) 
	{
		// create mesh for drawing map:
		mMapMesh.primitive(Graphics::QUADS);
		mMapMesh.color(1,1,1,1);
		int mapSteps =100;
		double step = 1./mapSteps;
		for (double x=0; x<=1.; x+=step) {
			for (double y=0; y<=1.; y+=step) {
				drawMapVertex(x,		y);
				drawMapVertex(x,		y+step);
				drawMapVertex(x+step,	y+step);
				drawMapVertex(x+step,	y);
			}
		}
	}
	
	virtual ~CubeMapTexture() {}
	
	virtual void onCreate() {
		// create cubemap texture:
		// RGBA8 Cubemap texture, 24 bit depth texture, mResolution x mResolution
		glGenTextures(1, (GLuint *)&mID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		// NULL means reserve texture memory, but texels are undefined
		for (int i=0; i<6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA8, mResolution, mResolution, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		}
				
		// clean up:
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		
		//printf("created CubeMapTexture %dx%d\n", mResolution, mResolution);
	}
	
	virtual void onDestroy() {
		glDeleteTextures(1, (GLuint *)&mID);
		mID=0;
	}
	
	void bind() const {
		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
	}
	
	void unbind() const {
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}	
	
	unsigned resolution() const { return mResolution; }
	
	// useful for debugging
	// draws full cubemap in a cylindrical projection
	void drawMap(Graphics& gl, double x0=0., double y0=0., double x1=1., double y1=1.) {
		bind();
		gl.color(1, 1, 1, 1);
		gl.pushMatrix();
		gl.translate(x0, y0, 0);
		gl.scale((x1-x0), (y1-y0), 1.);
		gl.draw(mMapMesh);
		gl.popMatrix();
		unbind();
	}

protected:
	inline void drawMapVertex(double x, double y) {
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
		mMapMesh.texCoord	( v );
		mMapMesh.vertex	( x, y, 0);
	}
	
	unsigned mResolution;
	Mesh mMapMesh;
};

class CubeMapFBO : public CubeMapTexture {
public:
	CubeMapFBO(int resolution=1024) 
	:	CubeMapTexture(resolution),
		mFboId(0),
		mRboId(0),
		mClearColor(0)
	{}
	
	virtual ~CubeMapFBO() {}
	
	virtual void onCreate() {
		CubeMapTexture::onCreate();
		
		//-------------------------
		glGenFramebuffersEXT(1, &mFboId);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboId);
		//Attach one of the faces of the Cubemap texture to this FBO
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, id(), 0);

		
		glGenRenderbuffersEXT(1, &mRboId);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mRboId);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, mResolution, mResolution);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mRboId);
		
		//-------------------------
		//Does the GPU support current FBO configuration?
		GLenum status;
		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			printf("GPU does not support required FBO configuration\n");
			exit(0);
		}
		
		// cleanup:
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	
	virtual void onDestroy() {
		glDeleteRenderbuffersEXT(1, &mRboId);
		glDeleteFramebuffersEXT(1, &mFboId);
		mRboId = mFboId = 0;
		
		CubeMapTexture::onDestroy();
	}
	
	void capture(Graphics& gl, const Camera& cam, const Pose& pose, Drawable& draw) {
		
		Vec3d pos = pose.pos();
		Vec3d ux, uy, uz; 
		pose.unitVectors(ux, uy, uz);
		
		mProjection = Matrix4d::perspective(90, 1, cam.near(), cam.far());
		
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboId);
		for (int i=0; i<6; i++) {
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, id(), 0);
			
			gl.viewport(0, 0, resolution(), resolution());
			gl.clearColor(clearColor());
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			
			switch (i) {
				case 0:
					// GL_TEXTURE_CUBE_MAP_POSITIVE_X   
					mModelView = Matrix4d::lookAt(uz, uy, -ux, pos);
					break;
				case 1:
					// GL_TEXTURE_CUBE_MAP_NEGATIVE_X   
					mModelView = Matrix4d::lookAt(-uz, uy, ux, pos);
					break;
				case 2:
					// GL_TEXTURE_CUBE_MAP_POSITIVE_Y   
					mModelView = Matrix4d::lookAt(ux, -uz, uy, pos);
					break;
				case 3:
					// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y   
					mModelView = Matrix4d::lookAt(ux, uz, -uy, pos);
					break;
				case 4:
					// GL_TEXTURE_CUBE_MAP_POSITIVE_Z   
					mModelView = Matrix4d::lookAt(ux, uy, uz, pos);
					break;
				default:
					// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z   
					mModelView = Matrix4d::lookAt(-ux, uy, -uz, pos);
					break;
			}
			
			// apply camera transform:
			gl.pushMatrix(gl.PROJECTION);
			gl.loadMatrix(mProjection);
			gl.pushMatrix(gl.MODELVIEW);
			gl.loadMatrix(mModelView);
			
			draw.onDraw(gl);
			
			gl.popMatrix(gl.PROJECTION);
			gl.popMatrix(gl.MODELVIEW);
		}
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	
	Color clearColor() const { return mClearColor; }
	Matrix4d projection() const { return mProjection; }
	Matrix4d modelView() const { return mModelView; }
	GLuint fbo() { return mFboId; }
	GLuint rbo() { return mRboId; }

	CubeMapFBO& clearColor(const Color& c) { mClearColor = c; return *this; }
	
protected:
	GLuint mFboId, mRboId;
	Color mClearColor;
	Matrix4d mProjection;
	Matrix4d mModelView;
};

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

struct MyWindow : Window, public Drawable{

	CubeMapFBO mCubeFBO;
	
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
		mCubeFBO.validate();
		return true;
	}

	bool onFrame(){
		
		nav.step();
	
		int w = width();
		int h = height();
		
		Viewport vp(width(), height());
		
		// capture the scene:
		mCubeFBO.capture(gl, cam, nav, *this);

		// now draw the captured texture:		
		gl.viewport(vp.l, vp.b, vp.w, vp.h);
		gl.clearColor(mCubeFBO.clearColor());
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		
		// ortho for 2D, ranging from 0..1 in each axis:
		gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
		gl.modelView(Matrix4d::identity());
		gl.lighting(false);
		gl.blending(false);
		mCubeFBO.drawMap(gl);
		
		return true;
	}
	
	bool onDestroy() {
		mCubeFBO.destroy();
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
	win.create(Window::Dim(100, 0, 640, 480), "Cube Map FBO Example", 60);
	
	win.displayMode(win.displayMode() | DisplayMode::StereoBuf);

	win.add(new StandardWindowKeyControls);
	win.add(new NavInputControl(nav));

	MainLoop::start();
    return 0;
}
