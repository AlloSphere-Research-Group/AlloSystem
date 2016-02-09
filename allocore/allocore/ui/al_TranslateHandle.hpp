#ifndef __TRANSLATE_HANDLE_HPP__
#define __TRANSLATE_HANDLE_HPP__

#include "allocore/ui/al_Gnomon.hpp"

struct TranslateHandle {

  Vec3f newPos;
  bool hover[3] = {false,false,false};
  bool selected[3] = {false,false,false};

  TranslateHandle(){}

  void set(TranslateHandle &th){
    newPos.set(th.newPos);
    for(int i=0; i<3; i++){
      hover[i] = th.hover[i];
      selected[i] = th.selected[i];
    }
  }

  void draw(Graphics &g, Vec3f& pos, Pose& text, float scale){
    Gnomon::gnomon.drawAtPos(g, pos, text, 1);
    for(int i=0; i < 3; i++){
      if(hover[i]){
        Mesh &m = g.mesh();
        m.reset();
        m.primitive(g.LINES);
        m.vertex(pos+Vec3f(i==0,i==1,i==2)*100);
        m.vertex(pos+Vec3f(i==0,i==1,i==2)*-100);
        g.color(i==0,i==1,i==2);
        g.lineWidth(1);
        g.draw(m);
        // Gnomon::gnomon.drawAtPos(g, pos+Vec3f(i==0,i==1,i==2), text, 0.1);
      }
      if(selected[i]){
        // Gnomon::gnomon.drawAtPos(g, newPos, text, 0.1);
      }
    }
  }

  bool point(Rayd &r, Vec3f& pos){
    for(int i=0; i < 3; i++){
      if( r.intersectsSphere(pos+Vec3f(i==0,i==1,i==2), 0.1)){
        hover[i] = true;
        return true;
      } else hover[i] = false;
    }
    return false;
  }

  bool pick(Rayd &r, Vec3f& pos){
    for(int i=0; i < 3; i++){
      if( r.intersectsSphere(pos+Vec3f(i==0,i==1,i==2), 0.1)){
        selected[i] = true;
        return true;
      } else selected[i] = false;
    }
    return false;  
  }

  bool drag(Rayd &r, Vec3f& pos, Vec3f& newP){
    newP.set(pos);
    for(int i=0; i < 3; i++){
      if(selected[i]){
        float t = r.intersectPlane(pos, Vec3f(i==1,i==2,i==0));
        if(t > 0){
          newPos.set(pos);
          newPos[i] = r(t)[i]-1;
          newP.set(newPos);
          return true;
          // return newPos;
        } 
      }
    }
    return false;
    // return pos;
  }
  void unpick(){ 
    for(int i=0; i < 3; i++) selected[i] = false; 
  }
  
};


#endif
