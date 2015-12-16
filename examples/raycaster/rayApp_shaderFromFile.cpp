#include "alloutil/al_RayApp.hpp"

using namespace al;

struct MyApp : public RayApp {

  MyApp()
  {
    nav().pos(0,0,3.0);
  }
  
  virtual ~MyApp() {
  }

  virtual void initShader() override {
  }

  virtual void loadShaders() override {
    ShaderProgram *s = mShaderManager.addShaderFile("toroid", "toroid.vert", "toroid.frag");
    s->begin();
    s->uniform("pixelMap", 1);
    s->uniform("alphaMap", 2);
    s->end();

    printf ("Vendor: %s\n", glGetString (GL_VENDOR));
    printf ("Renderer: %s\n", glGetString (GL_RENDERER));
    printf ("Version: %s\n", glGetString (GL_VERSION));
    printf ("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));
  }

  virtual void onDraw(Graphics& g) override {

    ShaderProgram *s = mShaderManager.get("toroid");
    s->begin();
    
    sendUniforms(s);

    Viewport vp(width(), height());
    mOmni.draw(s, lens(), vp);
    s->end();
  }
  
  virtual void onAnimate(al_sec dt) override {
  }
  
  virtual void onSound(AudioIOData& io) override {
    while (io()) {
      io.out(0) = rnd::uniformS() * 0.02;
    }
  }
  
  virtual void sendUniforms(ShaderProgram* shaderProgram) override {
    shaderProgram->uniform("pos", nav().pos());
    shaderProgram->uniform("quat", nav().quat());
    shaderProgram->uniform("time", FPS::now());
  }
  
  virtual void onMessage(osc::Message& m) override {
  }
  
  virtual bool onKeyDown(const Keyboard& k) override {
    if(k.ctrl()) {
      switch(k.key()) {
        case '1': mOmni.mode(RayStereo::MONO); return false;
        case '2': mOmni.mode(RayStereo::SEQUENTIAL); return false;
        case '3': mOmni.mode(RayStereo::ACTIVE); return false;
        case '4': mOmni.mode(RayStereo::DUAL); return false;
        case '5': mOmni.mode(RayStereo::ANAGLYPH); return false;
        case '6': mOmni.mode(RayStereo::LEFT_EYE); return false;
        case '7': mOmni.mode(RayStereo::RIGHT_EYE); return false;
      }
    } else {
      if(k.key() == '1') {
        nav().pos().set(0.000000, 0.000000, 3.000000);
        nav().quat().set(1.000000, 0.000000, 0.000000, 0.000000);
      }
      else if(k.key() == '2') {
        nav().pos().set(0.647900, -0.344193, 0.861521);
        nav().quat().set(0.977944, -0.119045, -0.164871, 0.047663);
      }
      else if(k.key() == '3') {
        nav().pos().set(0.119933, 0.338736, 0.335348);
        nav().quat().set(0.899555, -0.436596, 0.001055, -0.013571);
      }
      else if(k.key() == '4') {
        nav().pos().set(-0.464406, -0.093565, -0.479541);
        nav().quat().set(0.624573, -0.730013, -0.073427, -0.267580);
      }
    }
    
    return true;
  }
};

int main(int argc, char * argv[]) {
  MyApp().start();
  return 0;
}