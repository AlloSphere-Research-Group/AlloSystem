#ifndef __TRANSLATE_HANDLE_HPP__
#define __TRANSLATE_HANDLE_HPP__

#include "allocore/ui/al_Gnomon.hpp"
#include "allocore/ui/al_Pickable.hpp"

namespace al {

struct TranslateHandle : PickableBase {

  Vec3f translate;
  bool hover[3] = {false,false,false};
  bool selected[3] = {false,false,false};

  Gnomon gnomon;

  TranslateHandle(){}

  void set(TranslateHandle &th){
    pose.set(th.pose);
    translate.set(th.translate);
    for(int i=0; i<3; i++){
      hover[i] = th.hover[i];
      selected[i] = th.selected[i];
    }
  }

  void draw(Graphics &g){
    gnomon.drawAtPos(g, pose.pos(), Pose(), 1);
    for(int i=0; i < 3; i++){
      if(hover[i]){
        Mesh &m = g.mesh();
        m.reset();
        m.primitive(g.LINES);
        m.vertex(pose.pos()+Vec3f(i==0,i==1,i==2)*100);
        m.vertex(pose.pos()+Vec3f(i==0,i==1,i==2)*-100);
        g.color(i==0,i==1,i==2);
        g.lineWidth(1);
        g.draw(m);
      }
      // if(selected[i]){}
    }
  }

  double intersect(Rayd &r){
    // for(int i=0; i < 3; i++){
    //   double t = r.intersectSphere(pose.pos()+Vec3f(i==0,i==1,i==2), 0.1);
    //   if(t > 0) return t;
    // }
    return 0;
  };

  bool onPoint(Rayd &r, double t, bool child){
    if(child) return true;
    for(int i=0; i < 3; i++){
      if( r.intersectsSphere(pose.pos()+Vec3f(i==0,i==1,i==2), 0.1)){
        hover[i] = true;
        return true;
      } else hover[i] = false;
    }
    return false;
  }

  bool onPick(Rayd &r, double t, bool child){
    if(child) return true;
    for(int i=0; i < 3; i++){
      if( r.intersectsSphere(pose.pos()+Vec3f(i==0,i==1,i==2), 0.1)){
        selected[i] = true;
        return true;
      } else selected[i] = false;
    }
    return false;  
  }

  bool onDrag(Rayd &r, double t, bool child){
    if(child) return true;
    for(int i=0; i < 3; i++){
      if(selected[i]){
        float t = r.intersectPlane(pose.pos(), Vec3f(i==1,i==2,i==0));
        if(t > 0){
          translate.zero();
          translate[i] = r(t)[i] - 1 - pose.pos()[i];

          if(parent){
            Vec3f dir;
            switch(i){
              case 0: dir = parent->pose.ux(); break;
              case 1: dir = parent->pose.uy(); break;
              case 2: dir = parent->pose.uz(); break;
            }
            parent->pose.pos() += dir*translate[i];

          } else pose.pos()[i] += translate[i];
          return true;
        } 
      }
    }
    return false;
  }
  bool onUnpick(Rayd &r, double t, bool child){ 
    for(int i=0; i < 3; i++) selected[i] = false; 
    return false;
  }
  
};

} // ::al::

#endif
