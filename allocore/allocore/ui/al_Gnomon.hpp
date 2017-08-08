#ifndef INCLUDE_AL_GNOMON_HPP
#define INCLUDE_AL_GNOMON_HPP

#include "allocore/graphics/al_Font.hpp"

namespace al {

struct Gnomon {
  Mesh gnomonMesh;
  Mesh arrowMesh;

  // static Gnomon gnomon;

  Color colors[3];
  const char* labels[3];

  Gnomon() { 
    colors[0]=RGB(1,0,0);
    colors[1]=RGB(0,1,0);
    colors[2]=RGB(0,0,1);
    labels[0]="x";
    labels[1]="y";
    labels[2]="z";
    addPrism(arrowMesh, 0.0, 0.025, .1, 10, 0);
    gnomonMesh.primitive(Graphics::LINES);
    // x line
    gnomonMesh.vertex(0,0,0);
    gnomonMesh.color(colors[0]);
    gnomonMesh.vertex(1,0,0);
    gnomonMesh.color(colors[0]);
    // y line
    gnomonMesh.vertex(0,0,0);
    gnomonMesh.color(colors[1]);
    gnomonMesh.vertex(0,1,0);
    gnomonMesh.color(colors[1]);
    // z line
    gnomonMesh.vertex(0,0,0);
    gnomonMesh.color(colors[2]);
    gnomonMesh.vertex(0,0,1);
    gnomonMesh.color(colors[2]);
  }

  void draw(Graphics &g){
    g.draw(gnomonMesh);
    drawArrows(g);
  }

  // floating gnomon in front of camera
  void drawFloating(Graphics &g, Pose pose, float scale){
    g.pushMatrix();
      g.polygonMode(Graphics::LINE);
      Vec3d gnomZ = pose.pos() - (pose.uz()*2); // put in front of camera
      gnomZ -= (pose.uy() * .4); // translate to the bottom
      gnomZ -= (pose.ux() * .5); // translate to the left
      g.translate(gnomZ);
      g.scale(scale);
      g.draw(gnomonMesh);
      drawArrows(g);
      // drawLabels(g, pose, .005, gnomZ);
    g.popMatrix();
  }

  // Gnomon at any position
  void drawAtPos(Graphics &g, Vec3f pos, Pose cam_pose, float scale){
    g.pushMatrix();
      g.translate(pos);
      g.scale(scale);
      g.lineWidth(2);
      g.draw(gnomonMesh);
      // drawLabels(g, cam_pose, .002, pos);
      drawArrows(g);
    g.popMatrix();
  }

  void drawOrigin(Graphics &g, Pose cam_pose, float scale) {
    drawAtPos(g, Vec3f(0,0,0), cam_pose, scale);
  }

  // Gnomon at any pose
  void drawAtPose(Graphics &g, Pose pose, Pose cam_pose, float scale){
    g.pushMatrix();
      g.translate(pose.pos());
      g.rotate(pose.quat());
      g.scale(scale);
      g.lineWidth(2);
      g.draw(gnomonMesh);
      drawArrows(g);
      // drawLabels(g, cam_pose, .002);
    g.popMatrix();
  }

  void drawArrows(Graphics &g){
    Quatf q;
    for (int i = 0; i < 3; i++){
      glPushAttrib(GL_CURRENT_BIT);
      g.polygonMode(Graphics::FILL);
      g.pushMatrix();
        g.color(colors[i]);
        g.translate(gnomonMesh.vertices()[(i*2)+1]);
        switch(i){
          case 0: q.fromEuler(M_PI/2,0,0); break;
          case 1: q.fromEuler(0,-M_PI/2,0); break;
          case 2: q.fromEuler(0,0,0); break;
        }
        g.rotate(q);
        g.draw(arrowMesh);
      g.popMatrix();
      glPopAttrib();
    }
  }

  void drawLabels(Graphics &g, Font &font, Pose cam_pose, float scale, Vec3f offset = Vec3f(0,0,0)){
    // XYZ labels
    g.polygonMode(Graphics::FILL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5);
    for (int i = 0; i < 3; i++){
      g.pushMatrix();
        Vec3f xform = gnomonMesh.vertices()[(i*2)+1];
        Vec3d forward = Vec3d(cam_pose.pos() - xform - offset).normalize();
        Quatd rot = Quatd::getBillboardRotation(forward, cam_pose.uu());

        g.translate(xform);
        g.rotate(rot);
        g.scale(scale);

        g.color(colors[i]);
        font.render(g,labels[i]);
      g.popMatrix();
    }
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
  }

};

} // ::al::
// Gnomon Gnomon::gnomon;

#endif
