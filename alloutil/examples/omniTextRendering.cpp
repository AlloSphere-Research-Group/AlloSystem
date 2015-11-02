#include "alloutil/al_OmniApp.hpp"
#include "allocore/graphics/al_Font.hpp"
#include <iostream>

using namespace al;
using namespace std;

struct MyApp : OmniApp {
  Mesh mesh;
  Light light;
  Font font1;

  MyApp() :font1("../../allocore/share/fonts/VeraMoIt.ttf", 20) {
    mesh.primitive(Graphics::TRIANGLES);
    addSphere(mesh, 1.0, 32, 32);
    for (int i = 0; i < mesh.vertices().size(); ++i) {
      float f = (float)i / mesh.vertices().size();
      mesh.color(Color(HSV(f, 1 - f, 1), 1));
    }
  }

  virtual ~MyApp() {}

  virtual void onDraw(Graphics& g) {
    g.polygonMode(Graphics::LINE);
    g.draw(mesh);

    // Depth test so transparent faces are sorted properly
    g.polygonMode(Graphics::FILL);
    glAlphaFunc(GL_GREATER, 0.5);
    glEnable(GL_ALPHA_TEST);
    shader().uniform("texture", 1.0);
    
    for (int i = 0; i < mesh.vertices().size(); i++){
      g.pushMatrix();
        Vec3d xform = mesh.vertices()[i];
        g.translate(xform);
        g.scale(.001);
        
        // getLookRotation takes a normalized vector from source to destination
        // and an up vector, in this case we want the camera's up vector
        Vec3d forward = Vec3d(pose.pos() - xform).normalize();
        Quatd rot = Quatd::getBillboardRotation(forward, pose.uu());
        g.rotate(rot);

        stringstream sstream;
        sstream << float(i);
        string temp_str = sstream.str();
        const char* text = temp_str.c_str();
        font1.render(g, text);
      g.popMatrix();
    }
    glDisable(GL_ALPHA_TEST);
    shader().uniform("texture", 0.0);
  }

  virtual void onAnimate(al_sec dt) {
    pose = nav();
  }


  virtual void onMessage(osc::Message& m) {
    OmniApp::onMessage(m);
  }

  virtual bool onKeyDown(const Keyboard& k) { return true; }

  std::string fragmentCode() {
    return R"(
      uniform float lighting;
      uniform float texture;
      uniform sampler2D texture0;
      varying vec4 color;
      varying vec3 normal, lightDir, eyeVec;

      void main() {
        vec4 colorMixed;
        vec4 textureColor = texture2D(texture0, gl_TexCoord[0].st);
        if( texture > 0.0){
          colorMixed = mix(color, textureColor, texture);
        }else{
          colorMixed = color;
        }

        vec4 final_color = colorMixed * gl_LightSource[0].ambient;
        vec3 N = normalize(normal);
        vec3 L = lightDir;
        float lambertTerm = max(dot(N, L), 0.0);
        final_color += gl_LightSource[0].diffuse * colorMixed * lambertTerm;
        vec3 E = eyeVec;
        vec3 R = reflect(-L, N);
        float spec = pow(max(dot(R, E), 0.0), 0.9 + 1e-20);
        final_color += gl_LightSource[0].specular * spec;
        gl_FragColor = mix(colorMixed, final_color, lighting);

        // This sets alpha to 0 for everything but the letter
/*!*/   if (texture > 0.0) gl_FragColor.a = textureColor.r;
      }
    )";
  }
};

int main(int argc, char* argv[]) {
  MyApp().start();
  return 0;
}
