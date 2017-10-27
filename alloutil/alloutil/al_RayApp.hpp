#ifndef AL_RAYAPP_H
#define AL_RAYAPP_H

#include "allocore/al_Allocore.hpp"
#include "allocore/io/al_Window.hpp"
#include "allocore/protocol/al_OSC.hpp"
#include "alloutil/al_FPS.hpp"
#include "alloutil/al_RayStereo.hpp"
#include "alloutil/al_ShaderManager.hpp"

#define PORT_TO_DEVICE_SERVER (12000)
#define PORT_FROM_DEVICE_SERVER (PORT_TO_DEVICE_SERVER + 1)
#define DEVICE_SERVER_IP_ADDRESS "BOSSANOVA"

namespace al {
  
  class RayApp : public Window,
                 public FPS,
                 public osc::PacketHandler {
  public:
    RayApp(std::string name = "rayapp", bool slave=false, unsigned int recvPort=PORT_FROM_DEVICE_SERVER);
    virtual ~RayApp();
    
    void start();
    
    virtual bool onCreate();
    virtual bool onDestroy();
    virtual bool onFrame();
    virtual void onDraw(Graphics& gl) {}
    virtual void onAnimate(al_sec dt) {}
    virtual void onSound(AudioIOData& io) {}
    virtual void onMessage(osc::Message& m);
    
    void initWindow(const Window::Dim& dims = Window::Dim(800, 600),
                    const std::string title = "RayApp",
                    double fps = 60,
                    Window::DisplayMode mode = Window::DEFAULT_BUF);
    
    void initAudio(double audioRate=44100,
                   int audioBlockSize=256);
    
    void initAudio(std::string devicename,
                   double audioRate,
                   int audioBlockSize,
                   int audioInputs,
                   int audioOutputs);
    
    const Graphics& graphics() const { return mGraphics; }
    Graphics& graphics(){ return mGraphics; }
    
    const AudioIO& audioIO() const { return mAudioIO; }
    AudioIO& audioIO(){ return mAudioIO; }
    
    const Lens& lens() const { return mLens; }
    Lens& lens() { return mLens; }
    
    const Nav& nav() const { return mNav; }
    Nav& nav() { return mNav; }
    
    RayStereo& omni() { return mOmni; }

    void initOmni(std::string path = "");

    bool omniEnable() const { return bOmniEnable; }
    void omniEnable(bool b) { bOmniEnable = b; }

    ShaderManager& sm() { return mShaderManager; }
    
    virtual void initShader();
    virtual void loadShaders();
    virtual void sendUniforms(ShaderProgram* shaderProgram);
    
    const std::string&	hostName() const { return mHostName; }
    const std::string& name() const { return mName; }
    RayApp&	name(const std::string& v) { mName=v; return *this; }
    
    osc::Recv& oscRecv(){ return mOSCRecv; }
    osc::Send& oscSend(){ return mOSCSend; }
    
    void sendHandshake();
    void sendDisconnect();
    
    virtual std::string	vertexCode();
    virtual std::string	fragmentCode();
    
  protected:
    
    AudioIO mAudioIO;
    RayStereo mOmni;
    
    Lens mLens;
    Graphics mGraphics;
    
    ShaderManager mShaderManager;

    // control
    Nav mNav;
    NavInputControl mNavControl;
    StandardWindowKeyControls mStdControls;
    
    double mNavSpeed, mNavTurnSpeed;
    
    osc::Recv mOSCRecv;
    osc::Send mOSCSend;
    
    std::string mName;
    std::string mHostName;
    
    bool bOmniEnable, bSlave, bShaderLoaded;
    
    static void AppAudioCB(AudioIOData& io);
  };
  
  
  // INLINE IMPLEMENTATION //
  
  inline RayApp::RayApp(std::string name, bool slave, unsigned int recvPort)
  :	mNavControl(mNav),
	mOSCRecv(recvPort),
	mOSCSend(PORT_TO_DEVICE_SERVER, DEVICE_SERVER_IP_ADDRESS),
	bSlave(slave),
  mOmni(2048)
  {
    bOmniEnable = true;
    bShaderLoaded = false;
    mHostName = Socket::hostName();
    mName = name;
    
    mNavSpeed = 0.1;
    mNavTurnSpeed = 0.02;
    
    // default for omniapp: lens().near(0.01).far(40).eyeSep(0.03);
    // default for brain: lens().near(0.03).far(100).fovy(73.5).eyeSep(-0.02);
    lens().near(0.1).far(100).eyeSep(0.001);
    nav().smooth(0.8);
    
    Window::append(mStdControls);
    initWindow();
    initOmni();
    
    if (!bSlave) {
      Window::append(mNavControl);
      initAudio();
      
      oscRecv().bufferSize(32000);
      oscRecv().handler(*this);
      sendHandshake();
    }
  }
  
  inline RayApp::~RayApp() {
    if (!bSlave) sendDisconnect();
  }
  
  inline void RayApp::initWindow(const Window::Dim& dims,
                                 const std::string title,
                                 double fps,
                                 Window::DisplayMode mode) {
    Window::dimensions(dims);
    Window::title(title);
    Window::fps(fps);
    Window::displayMode(mode);
  }

  inline void RayApp::initOmni(std::string path) {
    printf("Searching for config file..\n");
    omni().loadConfig(path, mHostName);
  }
  
  inline void RayApp::initAudio(double audioRate, int audioBlockSize) {
    audioIO().callback = AppAudioCB;
    audioIO().user(this);
    audioIO().framesPerSecond(audioRate);
    audioIO().framesPerBuffer(audioBlockSize);
  }
  
  inline void RayApp::initAudio(std::string devicename,
                                double audioRate,
                                int audioBlockSize,
                                int audioInputs,
                                int audioOutputs) {
    AudioDevice indev(devicename, AudioDevice::INPUT);
    AudioDevice outdev(devicename, AudioDevice::OUTPUT);
    indev.print();
    outdev.print();
    audioIO().deviceIn(indev);
    audioIO().deviceOut(outdev);
    audioIO().channelsOut(audioOutputs);
    audioIO().channelsIn(audioInputs);
    initAudio(audioRate, audioBlockSize);
  }
  
  inline void RayApp::sendHandshake() {
    oscSend().send("/handshake", name(), oscRecv().port());
  }
  
  inline void RayApp::sendDisconnect() {
    oscSend().send("/disconnectApplication", name());
  }
  
  inline void RayApp::start() {
    if (omni().activeStereo()) {
      Window::displayMode(Window::displayMode() | Window::STEREO_BUF);
    }
    
    create();
    
    if (omni().fullScreen()) {
      fullScreen(true);
      cursorHide(true);
    }
    
    if (!bSlave) {
      if (oscSend().opened()) sendHandshake();
      audioIO().start();
    }
    
    Main::get().start();
  }
  
  inline bool RayApp::onCreate() {
    
    omni().onCreate();
    
    return true;
  }
  
  inline bool RayApp::onDestroy() {
    sm().destroy();

    return true;
  }

  inline void RayApp::loadShaders(){
    std::cout << "loading Shaders" << std::endl;
    sm().vertLibCode = "#version 120\n";
    sm().addShaderString("default", vertexCode(), fragmentCode());
  }
  
  // for initializing parameters before compiling shader
  inline void RayApp::initShader() {
  }
  
  // basic uniforms used in the shader. override it on user code
  inline void RayApp::sendUniforms(ShaderProgram* shaderProgram) {
  }
  
  inline bool RayApp::onFrame() {
    if(!bShaderLoaded) {
      initShader();
      loadShaders();
      bShaderLoaded = true;
    }
    if(sm().poll()) loadShaders();

    FPS::onFrame();

    if(frame % 60 == 0) {
      printf("FPS: %03.6f\n", FPS::fps());
//      nav().print();
    }

    while(oscRecv().recv()) {}
    
    nav().step();
    
    onAnimate(dt);
    
    onDraw(graphics());

    return true;
  }

  
  
  inline void RayApp::onMessage(osc::Message& m) {
    float x;
    if (m.addressPattern() == "/mx") {
      m >> x;
      nav().moveR(-x * mNavSpeed);
      
    } else if (m.addressPattern() == "/my") {
      m >> x;
      nav().moveU(x * mNavSpeed);
      
    } else if (m.addressPattern() == "/mz") {
      m >> x;
      nav().moveF(x * mNavSpeed);
      
    } else if (m.addressPattern() == "/tx") {
      m >> x;
      nav().spinR(x * -mNavTurnSpeed);
      
    } else if (m.addressPattern() == "/ty") {
      m >> x;
      nav().spinU(x * mNavTurnSpeed);
      
    } else if (m.addressPattern() == "/tz") {
      m >> x;
      nav().spinF(x * -mNavTurnSpeed);
      
    } else if (m.addressPattern() == "/home") {
      nav().home();
      
    } else if (m.addressPattern() == "/halt") {
      nav().halt();
      
    }
  }
  
  inline std::string	RayApp::vertexCode() {
    return R"(
                        varying vec2 T;
                        void main(void) {
                          // pass through the texture coordinate (normalized pixel):
                          T = vec2(gl_MultiTexCoord0);
                          gl_Position = vec4(T*2.-1., 0, 1);
                        }
                        )";
  }
  
  inline std::string RayApp::fragmentCode() {
    return R"(
                        uniform sampler2D pixelMap;
                        uniform sampler2D alphaMap;
                        uniform vec4 quat;
                        uniform vec3 pos;
                        uniform float eyesep;
                        varying vec2 T;
                        
                        float accuracy = 1e-3;
                        float stepsize = 0.02;
                        float maxval = 10.0;
                        float normalEps = 1e-5;
                        
                        // q must be a normalized quaternion
                        vec3 quat_rotate(in vec4 q, in vec3 v) {
                          // return quat_mul(quat_mul(q, vec4(v, 0)), quat_conj(q)).xyz;
                          // reduced:
                          vec4 p = vec4(
                                        q.w*v.x + q.y*v.z - q.z*v.y,  // x
                                        q.w*v.y + q.z*v.x - q.x*v.z,  // y
                                        q.w*v.z + q.x*v.y - q.y*v.x,  // z
                                        -q.x*v.x - q.y*v.y - q.z*v.z   // w
                                        );
                          return vec3(
                                      -p.w*q.x + p.x*q.w - p.y*q.z + p.z*q.y,  // x
                                      -p.w*q.y + p.y*q.w - p.z*q.x + p.x*q.z,  // y
                                      -p.w*q.z + p.z*q.w - p.x*q.y + p.y*q.x   // z
                                      );
                        }
                        
                        float evalAt(vec3 at) {
                          // A torus. To be used with ray marching.
                          // See http://www.freigeist.cc/gallery.html .
                          float R = 1.0;
                          float r = 0.5;
                          
                          R *= R;
                          r *= r;
                          
                          float t = dot(at, at) + R - r;
                          
                          return t * t - 4.0 * R * dot(at.xy, at.xy);
                        }
                        
                        bool findIntersection(in vec3 orig, in vec3 dir, inout vec3 hitpoint, inout vec3 normal) {
                          // Raymarching with fixed initial step size and final bisection.
                          // The object has to define evalAt().
                          float cstep = stepsize;
                          float alpha = cstep;
                          
                          vec3 at = orig + alpha * dir;
                          float val = evalAt(at);
                          bool sit = (val < 0.0);
                          
                          alpha += cstep;
                          
                          bool sitStart = sit;
                          
                          while (alpha < maxval)
                          {
                            at = orig + alpha * dir;
                            val = evalAt(at);
                            sit = (val < 0.0);
                            
                            // Situation changed, start bisection.
                            if (sit != sitStart)
                            {
                              float a1 = alpha - stepsize;
                              
                              while (cstep > accuracy)
                              {
                                cstep *= 0.5;
                                alpha = a1 + cstep;
                                
                                at = orig + alpha * dir;
                                val = evalAt(at);
                                sit = (val < 0.0);
                                
                                if (sit == sitStart)
                                  a1 = alpha;
                              }
                              
                              hitpoint = at;
                              
                              // Finite difference thing. :)
                              normal.x = evalAt(at + vec3(normalEps, 0, 0));
                              normal.y = evalAt(at + vec3(0, normalEps, 0));
                              normal.z = evalAt(at + vec3(0, 0, normalEps));
                              normal -= val;
                              normal = normalize(normal);
                              
                              return true;
                            }
                            
                            alpha += cstep;
                          }
                          
                          return false;
                        }
                        
                        void main(){
                          vec3 light1 = pos + vec3(1, 2, 3);
                          vec3 light2 = pos + vec3(2, -3, 1);
                          vec3 color1 = vec3(0.3, 0.3, 1.0);
                          vec3 color2 = vec3(0.8, 0.8, 0.3);
                          vec3 ambient = vec3(0.3, 0.3, 0.3);
                          
                          // pixel location (calibration space):
                          vec3 v = normalize(texture2D(pixelMap, T).rgb);
                          // ray direction (world space);
                          vec3 rd = quat_rotate(quat, v);
                          
                          // stereo offset:
                          // should reduce to zero as the nv becomes close to (0, 1, 0)
                          // take the vector of nv in the XZ plane
                          // and rotate it 90' around Y:
                          vec3 up = vec3(0, 1, 0);
                          vec3 rdx = cross(normalize(rd), up);
                          
                          //vec3 rdx = projection_on_plane(rd, up);
                          vec3 eye = rdx * eyesep;// * 0.02;
                          
                          // ray origin (world space)
                          vec3 ro = pos + eye;
                          
                          // initial eye-ray to find object intersection:
                          float mindt = 0.01;	// how close to a surface we can get
                          float mint = mindt;
                          float maxt = 50.;
                          float t=mint;
                          float h = maxt;
                          
                          // find object intersection:
                          vec3 p = ro + mint*rd;
                          vec3 normal;
                          if(!findIntersection(ro, rd, p, normal)) {
                            t = maxt;
                          }
                          
                          // lighting:
                          vec3 color = vec3(0, 0, 0);
                          
                          if (t<maxt) {
                            // compute ray to light source:
                            vec3 ldir1 = normalize(light1 - p);
                            vec3 ldir2 = normalize(light2 - p);
                            
                            // abs for bidirectional surfaces
                            float ln1 = max(0.,dot(ldir1, normal));
                            float ln2 = max(0.,dot(ldir2, normal));
                            
                            color = ambient + color1 * ln1 + color2 * ln2;
                          }
                          
                          color *= texture2D(alphaMap, T).rgb;
                          
                          gl_FragColor = vec4(color, 1);
                        }
                        )";
  }
  
  inline void RayApp::AppAudioCB(AudioIOData& io){
    RayApp& app = io.user<RayApp>();
    io.frame(0);
    app.onSound(io);
  }
  
}

#endif
