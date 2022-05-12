/*
Allocore Example: Mirror

Description:
This example demonstrates how to use a fbo to render a reflection to a texture.

Author:
Tim Wood, Nov. 2015
*/

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_EasyFBO.hpp"
using namespace al;

class MyApp : public App {
public:

  EasyFBO fbo{1024, 1024}; // constructor takes width,height of frame buffer
  Pose mirrorPose;

  Light light;      // Necessary to light objects in the scene
  Material material;    // Necessary for specular highlights
  Mesh surface, sphere; // Geometry to render
  double phase = 0.5;     // Animation phase

  MyApp(){

    // position and orientation of our mirror
    mirrorPose.pos(0,1.5,0.5);
    mirrorPose.faceToward(Vec3f(0,0,0.5));

    // position nav
    nav().pos(0,-2.2,0.5);
    nav().faceToward(Vec3f(0,0,0.5));

    // Create a circular wave pattern
    addSurface(surface, 64,64);
    for(int i=0; i<surface.vertices().size(); ++i){
      Mesh::Vertex& p = surface.vertices()[i];
      p.z = cos(p.mag()*4*M_PI)*0.1;
    }
    surface.color(RGB(1));

    // For all meshes we would like to light, we must generate normals.
    // This function is valid for TRIANGLES and TRIANGLE_STRIP primitives.
    surface.generateNormals();

    // Create a sphere to see the location of the light source.
    addSphere(sphere, 0.05);
    sphere.color(RGB(1));

    initWindow();
  }

  void onAnimate(double dt){
    // Set light position
    phase += 1./1800; if(phase > 1) phase -= 1;
    float x = cos(7*phase*2*M_PI);
    float y = sin(11*phase*2*M_PI);
    float z = cos(phase*2*M_PI)*0.5 + 0.6;

    light.pos(x,y,z);
  }

  // move scene draw into seperate method since we will be rendering it twice
  // once from the view of the reflection, second from our nav
  void drawScene(Graphics& g){
     // Render sphere at light position; this will not be lit
    g.lighting(false);
    g.pushMatrix(Graphics::MODELVIEW);
      g.translate(Vec3f(light.pos()));
      sphere.colors()[0] = light.diffuse();
      g.draw(sphere);
    g.popMatrix();

    // Set up light
    light.globalAmbient(RGB(0.1));  // Ambient reflection for all lights
    light.ambient(RGB(0));      // Ambient reflection for this light
    light.diffuse(RGB(1,1,0.5));  // Light scattered directly from light
    light.attenuation(1,1,0);   // Inverse distance attenuation
    //light.attenuation(1,0,1);   // Inverse-squared distance attenuation

    // Activate light
    light();

    // Set up material (i.e., specularity)
    material.specular(light.diffuse()*0.2); // Specular highlight, "shine"
    material.shininess(50);     // Concentration of specular component [0,128]

    // Activate material
    material();

    // Draw surface with lighting
    g.draw(surface);
  }


  void onDraw(Graphics& g){
    // pose representing the virtual camera we will use to render the fbo 
    Pose mirrorCam;

    // reflect position of nav relative to mirror across mirror plane normal, then add mirror position offset
    mirrorCam.pos() = (nav().pos() - mirrorPose.pos()).reflect(mirrorPose.uf()) ;
    mirrorCam.pos() += mirrorPose.pos();
    
    // face toward a point along reflected direction from nav to mirror across mirror plane normal, keep mirror up vector
    mirrorCam.faceToward(mirrorCam.pos() + (mirrorPose.pos()-nav().pos()).reflect(mirrorPose.uf()), mirrorPose.uu());

    // render scene to frame buffer
    // set fbo pose to be mirrorCam
    fbo.modelView() = mirrorCam.matrix();
	fbo.projection() = Matrix4d::perspective(45, 1, 0.001, 100);
	fbo.clearColor(RGB(0.2));
    fbo.draw(g, [&](){
      g.pushMatrix();
      drawScene(g); // draw scene to mirror fbo

      // draw nav as sphere in mirror fbo
      g.translate(nav().pos());
      g.draw(sphere);
      g.popMatrix();
    });

    // draw our mirror 
    g.lighting(false);
    g.pushMatrix();
    g.translate(mirrorPose.pos());
    g.rotate(mirrorPose.quat());
    // float aspect = stereo().viewport().aspect();
    // g.scale(aspect,1,1); // fix aspect ratio
    g.color(1,1,1);
    fbo.texture().quad(g, 2, 2, -1, -1);
    g.popMatrix();

    // draw the scene 
    drawScene(g);
  }
};

int main(){
  MyApp().start();
}
