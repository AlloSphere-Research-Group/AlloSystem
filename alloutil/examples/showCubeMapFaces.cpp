#include "alloutil/al_OmniApp.hpp"
#include "allocore/math/al_Mat.hpp"
using namespace al;

#include <map>
using namespace std;

struct MyApp : OmniApp {
  Mesh mesh;
  GLubyte* rgb;

  Mesh m0, m1, m2, m3, m4, m5;

  MyApp() {
    omni().configure(OmniStereo::NOBLEND);

    Color    red(1, 0, 0);
    Color  green(0, 1, 0);
    Color   blue(0, 0, 1);
    Color yellow(1, 1, 0);
    Color   cyan(0, 1, 1);
    Color   pink(1, 0, 1);

    m0.primitive(Graphics::TRIANGLE_STRIP);
    addSurface(m0, 2, 2);
    m0.color(red);
    m0.color(red);
    m0.color(red);
    m0.color(red);
    m0.translate(0, 0, -1);

    m1.primitive(Graphics::TRIANGLE_STRIP);
    addSurface(m1, 2, 2);
    m1.color(green);
    m1.color(green);
    m1.color(green);
    m1.color(green);
    m1.translate(0, 0, -1);

    m2.primitive(Graphics::TRIANGLE_STRIP);
    addSurface(m2, 2, 2);
    m2.color(blue);
    m2.color(blue);
    m2.color(blue);
    m2.color(blue);
    m2.translate(0, 0, -1);

    m3.primitive(Graphics::TRIANGLE_STRIP);
    addSurface(m3, 2, 2);
    m3.color(yellow);
    m3.color(yellow);
    m3.color(yellow);
    m3.color(yellow);
    m3.translate(0, 0, -1);

    m4.primitive(Graphics::TRIANGLE_STRIP);
    addSurface(m4, 2, 2);
    m4.color(cyan);
    m4.color(cyan);
    m4.color(cyan);
    m4.color(cyan);
    m4.translate(0, 0, -1);

    m5.primitive(Graphics::TRIANGLE_STRIP);
    addSurface(m5, 2, 2);
    m5.color(pink);
    m5.color(pink);
    m5.color(pink);
    m5.color(pink);
    m5.translate(0, 0, -1);

    // 0 GL_TEXTURE_CUBE_MAP_POSITIVE_X (1, 0, 0) red
    // 1 GL_TEXTURE_CUBE_MAP_NEGATIVE_X (0, 1, 0) green
    // 2 GL_TEXTURE_CUBE_MAP_POSITIVE_Y (0, 0, 1) blue
    // 3 GL_TEXTURE_CUBE_MAP_NEGATIVE_Y (1, 1, 0) yellow
    // 4 GL_TEXTURE_CUBE_MAP_POSITIVE_Z (0, 1, 1) cyan?
    // 5 GL_TEXTURE_CUBE_MAP_NEGATIVE_Z (1, 0, 1) pink?

    rgb = new GLubyte[3 * 2000 * 1600];
  }
  // omni_face

  virtual std::string vertexCode() {
    return AL_STRINGIFY(
      varying vec4 vy_CubeMapFaceColor;
      varying vec4 color;
      varying vec3 normal, lightDir, eyeVec;
      void main() {
        vy_CubeMapFaceColor =
          (omni_face == 0) ? vec4(1, 0, 0, 1) :
          (omni_face == 1) ? vec4(0, 1, 0, 1) :
          (omni_face == 2) ? vec4(0, 0, 1, 1) :
          (omni_face == 3) ? vec4(1, 1, 0, 1) :
          (omni_face == 4) ? vec4(0, 1, 1, 1) :
          vec4(1, 0, 1, 1);
        color = gl_Color;
        vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
        normal = gl_NormalMatrix * gl_Normal;
        vec3 V = vertex.xyz;
        eyeVec = normalize(-V);
        lightDir = normalize(vec3(gl_LightSource[0].position.xyz - V));
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = omni_render(vertex);
      }
    );
  }

  virtual std::string fragmentCode() {
    return AL_STRINGIFY(
      varying vec4 vy_CubeMapFaceColor;
      uniform float lighting;
      uniform float texture;
      uniform sampler2D texture0;
      varying vec4 color;
      varying vec3 normal, lightDir, eyeVec;
      void main() {

        vec4 colorMixed;
        if (texture > 0.0) {
          vec4 textureColor = texture2D(texture0, gl_TexCoord[0].st);
          colorMixed = mix(color, textureColor, texture);
        } else {
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
        gl_FragColor = mix(colorMixed, final_color, vy_CubeMapFaceColor * 0.0);
        //gl_FragColor = vy_CubeMapFaceColor;
      }
    );
  }

  virtual ~MyApp() {}

  virtual void onDraw(Graphics& g) {

    g.pushMatrix(); // x+
      g.rotate(-90, 0, 1, 0);
      g.draw(m0);
    g.popMatrix();

    g.pushMatrix(); // x-
      g.rotate(90, 0, 1, 0);
      g.draw(m1);
    g.popMatrix();

    g.pushMatrix(); // y+
      g.rotate(90, 1, 0, 0);
      g.draw(m2);
    g.popMatrix();

    g.pushMatrix(); // y-
      g.rotate(-90, 1, 0, 0);
      g.draw(m3);
    g.popMatrix();

    g.pushMatrix(); // z+
      g.rotate(180, 0, 1, 0);
      g.draw(m4);
    g.popMatrix();

    g.pushMatrix(); // z-

      g.draw(m5);
    g.popMatrix();


//    // take a snapshot of the viewport
//    //
//    glReadPixels(0, 0, width(), height(), GL_RGB, GL_UNSIGNED_BYTE, rgb);
//
//    map<unsigned, unsigned> colorCount;
//    cout << width() * height() * 3 << endl;
//    for (int i = 0; i < width() * height() * 3; i += 3) {
//      unsigned c = (rgb[i] << 16) | (rgb[i + 1] << 8) | rgb[i + 2];
//      if (colorCount.find(c) == colorCount.end())
//        colorCount[c] = 1;
//      else
//        colorCount[c]++;
//    }
//    cout << colorCount.size() << " colors found" << endl;
//    for (std::map<unsigned, unsigned>::iterator it = colorCount.begin(); it != colorCount.end(); ++it)
//      std::cout << it->first << " => " << it->second << '\n';
    // count the number of colors seen
    //
  }

  virtual void onAnimate(al_sec dt) {
    nav().pos(0, 0, 0);
    nav().quat().print();
  }
};

int main(int argc, char* argv[]) {
  MyApp().start();
  return 0;
}
