#ifndef __PICKABLE_HPP__
#define __PICKABLE_HPP__

#include "allocore/ui/al_BoundingBox.hpp"
#include "allocore/ui/al_Gnomon.hpp"
#include "allocore/ui/al_TranslateHandle.hpp"
#include "allocore/ui/al_RotateHandle.hpp"

struct Pickable {
  // Mesh *mesh;
  BoundingBox bb; // original bounding box
  BoundingBox aabb; // axis aligned bounding box (after pose/scale transforms)

  TranslateHandle handle;
  RotateHandle rhandle;

  Vec3f selectOffset;
  float selectDist;

  Pose pose;
  float scale;
  Pose oldPose;
  float oldScale;

  bool hover;
  bool selected;

  bool enable;

  // HUD
  bool hud_setup = false;
  Mesh hudMesh;
  Mesh hudLine;
  Font font1;

  Pickable() : scale(1), font1("data/Avenir-Medium.otf", 72){
    for (int i = 1; i <= 2; i++){
      hudLine.vertex(0,0,0);
      float Cd = (float)i/2;
      hudLine.color(Cd, Cd, Cd);
    }
    hudLine.primitive(Graphics::LINES);
  }

  Pickable(Mesh &mesh) : scale(1), font1("data/Avenir-Medium.otf", 72) {
    // mesh = m;
    Vec3f bbMin,bbMax;
    mesh.getBounds(bbMin,bbMax);
    bb.set(bbMin,bbMax);

    for (int i = 1; i <= 2; i++){
      hudLine.vertex(0,0,0);
      float Cd = (float)i/2;
      hudLine.color(Cd, Cd, Cd);
    }
    hudLine.primitive(Graphics::LINES);
  }

  ////////////////////////////////////////////////////////////////

  /// apply pickable pose transforms
  inline void pushMatrix(Graphics &g){
    g.pushMatrix();
    g.translate(pose.pos());
    g.rotate(pose.quat());
    g.scale(scale);
  }
  /// pop matrix.
  inline void popMatrix(Graphics &g){
    g.popMatrix();
  }

  /// draw everything, depending on hover/selected state
  void draw(Graphics &g, Mesh& m, Pose &text){
    pushMatrix(g);
    glPushAttrib(GL_CURRENT_BIT);
    if(selected){
      g.color(0,1,1);
      g.draw(bb.mesh);
      g.draw(bb.tics);
    } else if(hover){
      g.color(1,1,1);
      g.draw(bb.mesh);
      g.draw(bb.tics);
    } else {}
    g.draw(m);
    glPopAttrib();
    popMatrix(g);

    Gnomon::gnomon.drawAtPose(g, pose, text, 1);
    if(selected || hover){
      updateAABB();
      handle.draw(g, aabb.cen, text, 1);
      rhandle.draw(g, aabb.cen, 0.5);
    }
  }

  void drawGrid(Graphics &g){
    pushMatrix(g);
    if(selected || hover){
      g.lineWidth(2);
      g.draw(bb.gridMesh[1]);
      g.lineWidth(1);
      g.draw(bb.gridMesh[0]);
    } else {}
    popMatrix(g);
  }

  /// draw mesh
  void draw(Graphics &g, Mesh& m){
    pushMatrix(g);
    g.draw(m);
    popMatrix(g);
  }

  /// draw just the Bounding Box
  void drawBB(Graphics &g){
    if(!selected && !hover) return;
    pushMatrix(g);
    glPushAttrib(GL_CURRENT_BIT);
    if(selected) g.color(0,1,1);
    else if(hover) g.color(1,1,1);
    g.draw(bb.mesh);
    g.draw(bb.tics);
    glPopAttrib();
    popMatrix(g);
  }

  void drawBBLabels(Graphics &g, Pose cam_pose){
    if(!selected && !hover) return;
    g.pushMatrix();
    bb.drawLabels(g, cam_pose, pose, scale);
    g.popMatrix();
  }

  /// draw only Axis Aligned Bounding Box
  void drawAABB(Graphics &g){
    glPushAttrib(GL_CURRENT_BIT);
    if(!selected && !hover) return;
    if(selected) g.color(0,1,1);
    else if(hover) g.color(1,1,1);
    g.draw(aabb.mesh);
    g.draw(aabb.tics);
    glPopAttrib();
  }

  /// draw mesh origin Gnomon
  void drawOrigin(Graphics &g, Pose& cam_pose){
    Gnomon::gnomon.drawAtPose(g, pose, cam_pose, 1);
  }

  /// draw center of pickable Gnomon with translate/rotate handles
  void drawCenterHandle(Graphics &g, Pose& cam_pose){
    if(selected || hover){
      glPushAttrib(GL_CURRENT_BIT);
      updateAABB();
      handle.draw(g, aabb.cen, cam_pose, 1);
      rhandle.draw(g, aabb.cen, 0.5);
      glPopAttrib();
    }
  }

  // HUD
 inline void hudRotScale(Graphics& g){
    glPushAttrib(GL_CURRENT_BIT);
    g.rotate(pose.quat());
    g.scale(0.003);
    g.color(1,1,1);
    glPopAttrib();
 }

  void drawHUDText(Graphics &g, Pose& nav, Pose& target){
    g.pushMatrix();
      // Handle depth test so they're sorted and properly transparent
      g.polygonMode(Graphics::FILL);
      glAlphaFunc(GL_GREATER, 0.5);
      glEnable(GL_ALPHA_TEST);

      // Vec3f tx = (nav.pos() - (nav.uz()*2))*0.01; // push towards the camera slightly
      Vec3f tx = nav.ux() * -2;  // move "left"
      tx += nav.uy() * .35; // move "up"
      tx += pose.pos();
      g.translate(tx);

      // render position
      g.pushMatrix();
        hudRotScale(g);
        stringstream sstream;
        sstream << "translation: ";
        renderText(g, sstream.str());
      g.popMatrix();

      // line spacing. translate by this much for each line (after the first one)
      tx = (nav.uy() * -.25);

      g.translate(tx);
      g.pushMatrix();
        hudRotScale(g);
        Vec3f xyz = target.pos();
        sstream.str("");
        sstream << xyz.x << ", " << xyz.y << ", " << xyz.z;
        renderText(g, sstream.str());
      g.popMatrix();


      // render rotation
      g.translate(tx);
      g.pushMatrix();
        hudRotScale(g);
        sstream.str("");
        sstream << "rotation: ";
        renderText(g, sstream.str());
      g.popMatrix();

      g.translate(tx);
      g.pushMatrix();
        hudRotScale(g);
        Quatd euler = target.quat();
        sstream.str("");
        sstream << euler.x << ", " << euler.y << ", " << euler.z << ", " << euler.w;
        renderText(g, sstream.str());
      g.popMatrix();

    glDisable(GL_ALPHA_TEST);
    g.popMatrix();
  }

  void renderText(Graphics &g, string _text){
    string temp_str = _text;
    const char* text = temp_str.c_str();
    font1.render(g, text);
  }

  void drawLine(Graphics &g, Pose& target, Pose& nav){
    g.pushMatrix();
      Vec3f tx = (nav.ux() * -2) + (nav.uy() * 0.5);  // move to top left corner...
      tx += pose.pos();
      hudLine.vertices()[0] = target.pos();
      hudLine.vertices()[1] = tx;
      tx = Vec3f(nav.pos() - pose.pos()).normalize() * 0.01; // push away from the camera
      g.translate(-tx);
      g.draw(hudLine);
    g.popMatrix();
  }

  ///////////////////////////////////////////////////////////////////////

  /// handle pointer hover action
  bool point(Rayd &r){
    if(intersectsBB(r)){
      hover = true;
      handle.point(r, aabb.cen);
      rhandle.point(r, aabb.cen);
    } else hover = false;
    return hover;
  }

  /// handle pointer pick action
  bool pick(Rayd &r, Nav& nav){
    // if intersection occured store and offset and distance for moving model
    if(intersectsBB(r)){
      selected = true;
      oldPose.set(pose);

      float t = (nav.pos() - pose.pos()).mag();
      selectDist = t;
      selectOffset = pose.pos() - r(t);
      bool hit = handle.pick(r, aabb.cen);
      if(hit) selected = false;
      else{
        hit = rhandle.pick(r, aabb.cen);
        if(hit){
          selected = false;
        }
      }
    } else selected = false;
    return selected;
  }

  /// handle pointer drag action
  bool drag(Rayd &r, bool apply=true){
    // if previously selected then move
    if(selected){
      Vec3f newPos = r(selectDist) + selectOffset;
      if(apply) pose.pos().set(newPos);
      return true;
    } else {
      // Vec3f newPos = handle.drag(r, aabb.cen, newPos);
      Vec3f newPos;
      bool hit = handle.drag(r, aabb.cen, newPos);
      // if(hit) setCenter(newPos); // XXX this breaks why??
      setCenter(newPos);
      // else {
        // Quatf quat = rhandle.drag(r, aabb.cen, quat);
        Quatf quat;
        hit = rhandle.drag(r, aabb.cen, quat);
        if(hit){
          pose.quat().set(quat*oldPose.quat());
          newPos.set(aabb.cen);
          setCenter(newPos);
        }
      // }
      return hit;
    }
  }

  /// handle pointer unpick action
  void unpick(){
    selected = false;
    handle.unpick();
    rhandle.unpick();
  }

  ////////////////////////////////////////////////////////////////////////////

  /// set the pickable's center position
  void setCenter(Vec3f& pos){
    updateAABB();
    Vec3f offset = aabb.cen - pose.pos();
    pose.pos().set(pos - offset);
  }

  /// set pickable's orientation maintaining same center position
  void setQuat(Quatf& q){
    updateAABB();
    Vec3f cen;
    cen.set(aabb.cen);
    pose.quat().set(q);
    setCenter(cen);
  }

  /// intersect ray with pickable BoundingBox
  bool intersectsBB(Rayd &ray){
    Rayd r = transformRayLocal(ray);
    bool t = r.intersectsBox(bb.cen, bb.dim);
    // bool t = ray.intersectsBox(transformVecWorld(bb.cen,1), transformVecWorld(bb.dim,0));
    return t;
  }

  /// intersect ray with pickable AxisAlignedBoundingBox
  bool intersectsAABB(Rayd &ray){
    bool t = ray.intersectsBox(aabb.cen, aabb.dim);
    return t;
  }

  /// intersect ray with bounding sphere
  float intersectBoundingSphere(Rayd &ray){
    float t = ray.intersectSphere( transformVecWorld(bb.cen), bb.dim.mag()*scale/2);
    return t;
  }

  /// calculate Axis aligned bounding box from mesh bounding box and current transforms
  void updateAABB(){
    // thanks to http://zeuxcg.org/2010/10/17/aabb-from-obb-with-component-wise-abs/
    Matrix4d t,r,s;
    Matrix4d model = t.translate(pose.pos()) * r.fromQuat(pose.quat()) * s.scale(scale);
    Matrix4d absModel(model);
    for(int i=0; i<16; i++) absModel[i] = abs(absModel[i]);
    Vec4d cen = model.transform(Vec4d(bb.cen, 1));
    Vec4d dim = absModel.transform(Vec4d(bb.dim, 0));
    aabb.setCenterDim(cen.sub<3>(0),dim.sub<3>(0));
  }

  /// transform a ray in world space to local space
  Rayd transformRayLocal(Rayd &ray){
    Matrix4d t,r,s;
    Matrix4d model = t.translate(pose.pos()) * r.fromQuat(pose.quat()) * s.scale(scale);
    Matrix4d invModel = Matrix4d::inverse(model);
    Vec4d o = invModel.transform(Vec4d(ray.o, 1));
    Vec4d d = invModel.transform(Vec4d(ray.d, 0));
    return Rayd(o.sub<3>(0), d.sub<3>(0));
    // ray.o.set(o.sub<3>(0));
    // ray.d.set(d.sub<3>(0).normalize());
    // return ray;
  }

  /// transfrom a vector in local space to world space
  Vec3f transformVecWorld(Vec3f &v, float w=1){
    Matrix4d t,r,s;
    Matrix4d model = t.translate(pose.pos()) * r.fromQuat(pose.quat()) * s.scale(scale);
    Vec4d o = model.transform(Vec4d(v, w));
    return Vec3f(o.sub<3>(0));
  }
};

#endif
