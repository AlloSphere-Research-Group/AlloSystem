
#ifndef __PICKABLE_HPP__
#define __PICKABLE_HPP__

#include <vector>

#include "allocore/ui/al_Gnomon.hpp"
#include "allocore/ui/al_BoundingBox.hpp"

namespace al {


struct PickableState {
  bool hover, selected;
  Pose pose;
  Vec3f scale;
  
  PickableState(){
    pose = Pose();
    scale = Vec3f(1);
    hover = selected = false;
  }
};

struct PickableBase : virtual PickableState {
  PickableBase *parent = 0;
  std::vector<PickableBase *> children;
  bool alwaysTestChildren = true;

  // initial values, and previous values
  Pose pose0, prevPose;
  Vec3f scale0, prevScale;

  /// intersection test must be specified
  virtual double intersect(Rayd &r) = 0;

  /// override these callbacks
  virtual bool onPoint(Rayd &r, double t, bool child){return false;}
  virtual bool onPick(Rayd &r, double t, bool child){return false;}
  virtual bool onDrag(Rayd &r, double t, bool child){return false;}
  virtual bool onUnpick(Rayd &r, double t, bool child){return false;}
  
  /// do interaction on self and children, call onPoint callbacks
  virtual bool point(Rayd &r){
    bool child = false;  
    double t = intersect(r);
    if(t > 0.0 || alwaysTestChildren){
      for(int i=0; i < children.size(); i++){
        Rayd ray = transformRayLocal(r);
        child |= children[i]->point(ray);
      }
    }
    return onPoint(r,t,child);
  }

  /// do interaction on self and children, call onPick callbacks
  virtual bool pick(Rayd &r){
    bool child = false;  
    double t = intersect(r);
    if(t > 0.0 || alwaysTestChildren){
      for(int i=0; i < children.size(); i++){
        Rayd ray = transformRayLocal(r);
        child |= children[i]->pick(ray);
      }
    }
    return onPick(r,t,child);
  }

  /// do interaction on self and children, call onDrag callbacks
  virtual bool drag(Rayd &r){
    bool child = false;  
    double t = intersect(r);
    if(t > 0.0 || alwaysTestChildren){
      for(int i=0; i < children.size(); i++){
        Rayd ray = transformRayLocal(r);
        child |= children[i]->drag(ray);
      }
    }
    return onDrag(r,t,child);
  }
  
  /// do interaction on self and children, call onUnpick callbacks
  virtual bool unpick(Rayd &r){
    bool child = false;  
    double t = intersect(r);
    if(t > 0.0 || alwaysTestChildren){
      for(int i=0; i < children.size(); i++){
        Rayd ray = transformRayLocal(r);
        child |= children[i]->unpick(ray);
      }
    }
    return onUnpick(r,t,child);
  }

  virtual void draw(Graphics &g){}
  virtual void drawChildren(Graphics &g){
    pushMatrix(g);
    for(int i=0; i < children.size(); i++){
      children[i]->draw(g);
    }
    popMatrix(g);
  }

  bool intersects(Rayd &r){ return intersect(r) > 0.0; }
  bool intersectsChild(Rayd &r){
    bool child = false;
    for(int i=0; i < children.size(); i++){
      Rayd ray = transformRayLocal(r);
      child |= children[i]->intersects(ray);
    }
    return child;
  }

  void addChild(PickableBase &pickable){
    pickable.parent = this;
    children.push_back(&pickable);
  }

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

  /// transform a ray in world space to local space
  Rayd transformRayLocal(const Rayd &ray){
    Matrix4d t,r,s;
    Matrix4d model = t.translation(pose.pos()) * r.fromQuat(pose.quat()) * s.scaling(scale);
    Matrix4d invModel = Matrix4d::inverse(model);
    Vec4d o = invModel.transform(Vec4d(ray.o, 1));
    Vec4d d = invModel.transform(Vec4d(ray.d, 0));
    return Rayd(o.sub<3>(0), d.sub<3>(0));
  }

  /// transfrom a vector in local space to world space
  Vec3f transformVecWorld(const Vec3f &v, float w=1){
    Matrix4d t,r,s;
    Matrix4d model = t.translation(pose.pos()) * r.fromQuat(pose.quat()) * s.scaling(scale);
    Vec4d o = model.transform(Vec4d(v, w));
    return Vec3f(o.sub<3>(0));
  }  
  /// transfrom a vector in world space to local space
  Vec3f transformVecLocal(const Vec3f &v, float w=1){
    Matrix4d t,r,s;
    Matrix4d invModel = t.translation(pose.pos()) * r.fromQuat(pose.quat()) * s.scaling(scale);
    Vec4d o = invModel.transform(Vec4d(v, w));
    return Vec3f(o.sub<3>(0));
  }
};


/// Bounding Box Pickable
struct Pickable : PickableBase {
  Mesh *mesh; // pointer to mesh that is wrapped
  BoundingBox bb; // original bounding box
  BoundingBox aabb; // axis aligned bounding box (after pose/scale transforms)

  // used for moving pickable naturally
  Vec3f selectOffset;
  float selectDist;

  Pickable(){}
  Pickable(Mesh &m){set(m);}

  /// initialize bounding box;
  void set(Mesh &m){
    mesh = &m;
    bb.set(*mesh);
  }

  /// override base methods
  double intersect(Rayd &r){
    return intersectBB(r);
  }

  bool onPoint(Rayd &r, double t, bool child){
    if(t > 0.0){
      if(child){
        hover = false;
      } else {
        hover = true;
      }
    } else hover = false;
    return hover || child;
  }

  bool onPick(Rayd &r, double t, bool child){
    if(t > 0.0){
      if(child){
        selected = false;
      } else {
        prevPose.set(pose);
        selectDist = t;
        selectOffset = pose.pos() - r(t);
        selected = true;
      }
    } else selected = false;
    return selected || child;
  }
  
  bool onDrag(Rayd &r, double t, bool child){
    // if(t > 0.0){
      if(child){
        return true;
      } else if(selected){
        Vec3f newPos = r(selectDist) + selectOffset;
        pose.pos().set(newPos);
        return true;
      }
    // }
    return false;
  }

  bool onUnpick(Rayd &r, double t, bool child){
    if(!hover) selected = false;
    return false;
  }

  // draw
  void draw(Graphics &g){
    pushMatrix(g);
    glPushAttrib(GL_CURRENT_BIT);
    if(mesh) g.draw(*mesh);

    if(selected) g.color(0,1,1);
    else if(hover) g.color(1,1,1);
    g.draw(bb.mesh);
    g.draw(bb.tics);

    // drawLabels(g);
    glPopAttrib();
    popMatrix(g);
    drawChildren(g);
  }

  void drawMesh(Graphics &g){
    if(!mesh) return;
    pushMatrix(g);
    g.draw(*mesh);
    popMatrix(g);
  }
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

  void drawLabels(Graphics &g, Font& font, Pose &cam_pose){
    if(!selected && !hover) return;
    g.pushMatrix();
    bb.drawLabels(g, font, cam_pose, pose, scale.x);
    g.popMatrix();
  }

  void drawOrigin(Graphics &g){
    Gnomon gnomon;
    pushMatrix(g);
    gnomon.draw(g);
    popMatrix(g);
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
  double intersectBB(Rayd &ray){
    Rayd r = transformRayLocal(ray);
    return r.intersectBox(bb.cen, bb.dim);
  }

  /// intersect ray with pickable AxisAlignedBoundingBox
  double intersectAABB(Rayd &ray){
    return ray.intersectBox(aabb.cen, aabb.dim);
  }

  /// intersect ray with bounding sphere
  float intersectBoundingSphere(Rayd &ray){
    return ray.intersectSphere( transformVecWorld(bb.cen), (bb.dim*scale).mag()/2.0);
  }

  /// calculate Axis aligned bounding box from mesh bounding box and current transforms
  void updateAABB(){
    // thanks to http://zeuxcg.org/2010/10/17/aabb-from-obb-with-component-wise-abs/
    Matrix4d t,r,s;
    Matrix4d model = t.translation(pose.pos()) * r.fromQuat(pose.quat()) * s.scaling(scale);
    Matrix4d absModel(model);
    for(int i=0; i<16; i++) absModel[i] = abs(absModel[i]);
    Vec4d cen = model.transform(Vec4d(bb.cen, 1));
    Vec4d dim = absModel.transform(Vec4d(bb.dim, 0));
    aabb.setCenterDim(cen.sub<3>(0),dim.sub<3>(0));
  }

};

} // ::al

#endif
