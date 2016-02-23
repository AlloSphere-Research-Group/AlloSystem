#ifndef __ROTATE_HANDLE_HPP__
#define __ROTATE_HANDLE_HPP__

void addCircle(Mesh &m, float r, int n){
  double inc = M_2PI/n;
  double phase = 0.0;
  for(int i=0; i < n; i++){
    float x = cos(phase)*r;
    float y = sin(phase)*r;
    m.vertex(x,y,0);
    phase += inc;
  }
}

struct RotateHandle {

  Vec3f downPos, newPos;
  Vec3f downDir, newDir;
  Quatf quat;
  bool hover[3] = {false,false,false};
  bool selected[3] = {false,false,false};

  RotateHandle(){ quat = Quatf::identity(); }

  void set(RotateHandle &rh){
    downPos.set(rh.downPos);
    downDir.set(rh.downDir);
    newPos.set(rh.newPos);
    newDir.set(rh.newDir);
    quat.set(rh.quat);
    for(int i=0; i<3; i++){
      hover[i] = rh.hover[i];
      selected[i] = rh.selected[i];
    }
  }

  void draw(Graphics &g, Vec3f& pos, float scale){
    Mesh &m = g.mesh();
    m.primitive(g.LINE_LOOP);
    m.reset();
    addCircle(m,scale,30);

    Quatf q;
    for (int i = 0; i < 3; i++){
      g.pushMatrix();
        g.translate(pos);
        g.color(i==0,i==1,i==2);
        switch(i){
          case 0: q.fromEuler(M_PI/2,0,0); break;
          case 1: q.fromEuler(0,-M_PI/2,0); break;
          case 2: q.fromEuler(0,0,0); break;
        }
        g.rotate(q);
        if(hover[i]) g.lineWidth(3);
        else g.lineWidth(1);
        g.draw(m);
      g.popMatrix();
    }

    for(int i=0; i < 3; i++){
      // if(hover[i]){
      //   Mesh &m = g.mesh();
      //   m.reset();
      //   m.primitive(g.LINES);
      //   m.vertex(pos+Vec3f(i==0,i==1,i==2)*100);
      //   m.vertex(pos+Vec3f(i==0,i==1,i==2)*-100);
      //   g.color(i==0,i==1,i==2);
      //   g.lineWidth(1);
      //   g.draw(m);
      //   // Gnomon::gnomon.draw(g, pos+Vec3f(i==0,i==1,i==2), text, 0.1);
      // }
      if(selected[i]){
        m.reset();
        m.primitive(g.LINES);
        m.vertex(pos);
        m.vertex(pos+newDir);
        m.vertex(pos);
        m.vertex(pos+downDir);
        g.color(i==0,i==1,i==2);
        g.lineWidth(1);
        g.draw(m);
      }
    }
  }

  bool point(Rayd &r, Vec3f& pos){
    if(r.intersectsSphere(pos, 0.55)){
      float t = -1;
      float min = FLT_MAX;
      int minIdx = -1;
      for(int i=0; i < 3; i++){
        hover[i] = false;
        t = r.intersectCircle(pos, Vec3f(i==0,i==1,i==2), 0.55, 0.45);
        if(t > 0 && t < min){
          min = t;
          minIdx = i;
        }
      }
      if(minIdx >= 0){
        hover[minIdx] = true;
        return true;
      }
    } else for(int i=0; i<3; i++) hover[i] = false;
    return false;
  }

  bool pick(Rayd &r, Vec3f& pos){
    if(r.intersectsSphere(pos, 0.55)){
      float t = -1;
      float min = FLT_MAX;
      int minIdx = -1;
      for(int i=0; i < 3; i++){
        selected[i] = false;
        t = r.intersectCircle(pos, Vec3f(i==0,i==1,i==2), 0.55);
        if(t > 0 && t < min){
          min = t;
          minIdx = i;
        }
      }
      if(minIdx >= 0){
        selected[minIdx] = true;
        downDir.set(r(min)-pos);
        newDir.set(r(min)-pos);
        return true;
      }
    }
    return false;  
  }

  bool drag(Rayd &r, Vec3f& pos, Quatf& newQuat){
    for(int i=0; i < 3; i++){
      if(selected[i]){
        float t = r.intersectPlane(pos, Vec3f(i==0,i==1,i==2));
        if(t > 0){
          newDir.set(r(t)-pos);
          quat = Quatf::getRotationTo(downDir.normalized(),newDir.normalized());
          newQuat.set(quat);
          return true;
          // return quat;
        } 
      }
    }
    return false;
    // return Quatf::identity();
  }
  void unpick(){ 
    for(int i=0; i < 3; i++) selected[i] = false; 
  }
  
};


#endif
