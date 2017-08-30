
#ifndef __BOUNDINGBOX_HPP__
#define __BOUNDINGBOX_HPP__

#include <string>
#include <sstream>
#include "allocore/graphics/al_Font.hpp"

namespace al {

struct BoundingBox {
  Vec3f min, max;
  Vec3f cen, dim;
  Mesh mesh, tics, gridMesh[2];
  float glUnitLength;
  float ticsPerUnit;

  BoundingBox() : glUnitLength(1.0), ticsPerUnit(10.0){}

  BoundingBox(const Vec3f &min_, const Vec3f &max_) : min(min_), max(max_){
    dim = max - min;
    Vec3f halfDim = dim / 2;
    cen = min + halfDim;
    getMesh();
    getTics();
  }

  // set bounds from mesh
  void set(const Mesh &mesh){
    Vec3f bbMin, bbMax;
    mesh.getBounds(bbMin, bbMax);
    set(bbMin, bbMax);
  }

  void set(const Vec3f &min_, const Vec3f &max_){
    min.set(min_);
    max.set(max_);
    dim = max - min;
    Vec3f halfDim = dim / 2;
    cen = min + halfDim;
    getMesh();
    getTics();
    getGrid();
  }

  void setCenterDim(const Vec3f &cen_, const Vec3f &dim_){
    cen.set(cen_);
    dim.set(dim_);
    min = cen - dim/2;
    max = cen + dim/2;
    getMesh();
    getTics();
    getGrid();
  }

  Mesh& getMesh(){
    mesh.reset();
    Vec3f halfDim = dim / 2;
    addWireBox(mesh, halfDim.x, halfDim.y, halfDim.z);
    mesh.translate(cen);
    return mesh;
  }

  void draw(Graphics &g, bool drawTics=false, bool drawGrid=false){
    g.draw(mesh);
    if(drawTics) g.draw(tics);
    if(drawGrid){
      g.draw(gridMesh[0]);
      g.draw(gridMesh[1]);
    }
  }

  void drawLabels(Graphics &g, Font &font, Pose cam_pose, Pose obj_pose, float obj_scale){
    g.pushMatrix();
      g.color(.35,.35,.35,1);
      g.lineWidth(1);
      drawLabelsOmni(g, font, cam_pose, obj_pose, obj_scale);
    g.popMatrix();
  }

  void drawLabelsOmni(Graphics &g, Font &font, Pose cam_pose, Pose obj_pose, float obj_scale){
    
    // Handle depth test so they're sorted and properly transparent
    g.polygonMode(Graphics::FILL);
    glAlphaFunc(GL_GREATER, 0.5);
    glEnable(GL_ALPHA_TEST);

    Vec3f halfDim = dim / 2;

    int longestEdge = (dim.x<dim.y)?dim.y:dim.x;
    longestEdge = (longestEdge<dim.z)?dim.z:longestEdge;
    longestEdge++;

    for (int i = 0; i < ((longestEdge*glUnitLength)/10); i++){
      for (int axis = 0; axis < 3; axis++){
        if ((axis == 0 && i <= (dim.x*glUnitLength)/10) || (axis == 1 && i <= (dim.y*glUnitLength)/10) || (axis==2 && i <= (dim.z*glUnitLength)/10)){
          g.pushMatrix();

            Vec3d xform;
            if (axis == 0)      xform = Vec3d(((i/glUnitLength)*10) - halfDim.x + (1/glUnitLength), -halfDim.y, 0);
            else if (axis == 1) xform = Vec3d(-halfDim.x, ((i/glUnitLength)*10) - halfDim.y - (1/glUnitLength), 0);
            else if (axis == 2) xform = Vec3d(-halfDim.x, -halfDim.y, ((i/glUnitLength)*10 + (1/glUnitLength)));
            
            xform = xform + Vec3f(cen.x, cen.y, cen.z-(dim.z/2));

            // move number to current draw pos
            Matrix4d t,r,s;
            Matrix4d model = t.translation(obj_pose.pos()) * r.fromQuat(obj_pose.quat()) * s.scaling(obj_scale);
            Vec4d m_xform = model.transform(Vec4d(xform, 1));
            xform = Vec3d(m_xform.x, m_xform.y, m_xform.z);

            g.translate(xform);
            g.color(1,1,1);
            g.scale(.0028);
            
            // vector from source to destination, camera up vector
            Vec3d forward = Vec3d(cam_pose.pos() - xform).normalize();
            Quatd rot = Quatd::getBillboardRotation(forward, cam_pose.uu());
            g.rotate(rot);

            std::stringstream sstream;
            if (i != 0) sstream << i*10; // only draw zero once
            else if (i == 0 && axis == 0) sstream << i*10; // only draw zero once
            std::string temp_str = sstream.str();
            const char* text = temp_str.c_str();
            font.render(g, text);

          g.popMatrix();
        }
      }
    }

    glDisable(GL_ALPHA_TEST);
  }

  // tic marks
  Mesh& getTics(){
    tics.reset();
    Vec3f halfDim = dim / 2;
    float ticLen = 0.1; // tic length multiplier
    // x tics
    for (int i = 0; i < dim.x*ticsPerUnit; i++){
      for (int z = 0; z < 2; z++){
        for (int y = -1; y < 2; y++){
          if (y != 0){
            float len = ticLen;
            if (i%5==0) len *= 2;
            if (i%10==0) len *= 1.5;
            tics.vertex((i/ticsPerUnit)-halfDim.x, y*halfDim.y, z*dim.z);
            tics.vertex((i/ticsPerUnit)-halfDim.x, y*halfDim.y-(len*y), z*dim.z);

            tics.vertex((i/ticsPerUnit)-halfDim.x, y*halfDim.y, z*dim.z);
            tics.vertex((i/ticsPerUnit)-halfDim.x, y*halfDim.y, z*dim.z-(((z*2)-1)*len));
          }
        }
      }
    }
    // y tics
    for (int i = 0; i < dim.y*ticsPerUnit; i++){
      for (int z = 0; z < 2; z++){
        for (int x = -1; x < 2; x++){
          if (x != 0){
            float len = ticLen;
            if (i%5==0) len *= 2;
            if (i%10==0) len *= 1.5;
            tics.vertex(x*halfDim.x, (i/ticsPerUnit)-halfDim.y, z*dim.z);
            tics.vertex(x*halfDim.x-(len*x), (i/ticsPerUnit)-halfDim.y, z*dim.z);

            tics.vertex(x*halfDim.x, (i/ticsPerUnit)-halfDim.y, z*dim.z);
            tics.vertex(x*halfDim.x, (i/ticsPerUnit)-halfDim.y, z*dim.z-(((z*2)-1)*len));
          }
        }
      }
    }
    // z tics
    for (int i = 0; i < dim.z*ticsPerUnit; i++){
      for (int z = 0; z < 2; z++){
        for (int x = -1; x < 2; x++){
          if (x != 0){
            float len = ticLen;
            if (i%5==0) len *= 2;
            if (i%10==0) len *= 1.5;
            tics.vertex(z*dim.x-halfDim.x, x*halfDim.y, (i/ticsPerUnit));
            tics.vertex(z*dim.x-halfDim.x, x*halfDim.y-(len*x), (i/ticsPerUnit));

            tics.vertex(z*dim.x-halfDim.x, x*halfDim.y, (i/ticsPerUnit));
            tics.vertex((z*dim.x-halfDim.x)-(((z*2)-1)*len), x*halfDim.y, (i/ticsPerUnit));
          }
        }
      }
    }
    tics.color(.4,.4,.4);
    tics.primitive(Graphics::LINES);
    tics.translate(cen.x,cen.y,cen.z-(dim.z/2));
    return tics;
  }

  // tic marks
  void getGrid(){
    Vec3f halfDim = dim / 2;

    for (int gridNum = 0; gridNum <= 1; gridNum++){
      gridMesh[gridNum].reset();
      int inc; // grid increment
      if (gridNum == 0) inc = 5;
      if (gridNum == 1) inc = 10;
        // x tics
        for (int x = 0; x <= dim.x*glUnitLength; x++){
          for (int z = 0; z <= dim.z*glUnitLength; z++){
            if ( x%inc==0 && z%inc==0 ) { 
              gridMesh[gridNum].vertex((x/glUnitLength)-halfDim.x, -halfDim.y, z/glUnitLength);
              gridMesh[gridNum].vertex((x/glUnitLength)-halfDim.x, halfDim.y, z/glUnitLength);
            }
            // manage end cases
            if ( z==0 && x%inc==0){
              gridMesh[gridNum].vertex((x/glUnitLength)-halfDim.x, -halfDim.y, dim.z);
              gridMesh[gridNum].vertex((x/glUnitLength)-halfDim.x, halfDim.y, dim.z);
            }
            if ( x==0 && z%inc==0){
              gridMesh[gridNum].vertex(halfDim.x, -halfDim.y, z/glUnitLength);
              gridMesh[gridNum].vertex(halfDim.x, halfDim.y, z/glUnitLength);
            }
          }
        }
        // y tics
        for (int y = 0; y <= dim.y*glUnitLength; y++){
          for (int z = 0; z <= dim.z*glUnitLength; z++){
            if ( y%inc==0 && z%inc==0 ) { 
              gridMesh[gridNum].vertex(-halfDim.x, (y/glUnitLength)-halfDim.y, z/glUnitLength);
              gridMesh[gridNum].vertex(halfDim.x, (y/glUnitLength)-halfDim.y, z/glUnitLength);
            }
            // manage end cases
            if ( z==0 && y%inc==0){
              gridMesh[gridNum].vertex(-halfDim.x, (y/glUnitLength)-halfDim.y, dim.z);
              gridMesh[gridNum].vertex(halfDim.x, (y/glUnitLength)-halfDim.y, dim.z);
            }
            if ( y==0 && z%inc==0){
              gridMesh[gridNum].vertex(-halfDim.x, halfDim.y, z/glUnitLength);
              gridMesh[gridNum].vertex(halfDim.x, halfDim.y, z/glUnitLength);
            }
          }
        }
        // z tics
        for (int y = 0; y <= dim.y*glUnitLength; y++){
          for (int x = 0; x <= dim.x*glUnitLength; x++){
            if ( x%inc==0 && y%inc==0 ) {
              gridMesh[gridNum].vertex((x/glUnitLength)-halfDim.x, (y/glUnitLength)-halfDim.y, 0);
              gridMesh[gridNum].vertex((x/glUnitLength)-halfDim.x, (y/glUnitLength)-halfDim.y, dim.z);
            }
            // manage end cases
            if ( y==0 && x%inc==0){
              gridMesh[gridNum].vertex((x/glUnitLength)-halfDim.x, halfDim.y, 0);
              gridMesh[gridNum].vertex((x/glUnitLength)-halfDim.x, halfDim.y, dim.z);
            }
            if ( x==0 && y%inc==0){
              gridMesh[gridNum].vertex(halfDim.x, (y/glUnitLength)-halfDim.y, 0);
              gridMesh[gridNum].vertex(halfDim.x, (y/glUnitLength)-halfDim.y, dim.z);
            }
          }
        }
        float Cd = .4 + (.2 * gridNum);
        gridMesh[gridNum].color(Cd,Cd,Cd,.7);
        gridMesh[gridNum].primitive(Graphics::LINES);
        gridMesh[gridNum].translate(cen.x,cen.y,cen.z-(dim.z/2));
      }
      // return gridMesh[2];
  }

};

} //::al

#endif