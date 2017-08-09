#include "alloutil/al_SliceViewer.hpp"

namespace al {

/*  //BACK-UP
bool SliceViewer::linePlaneIntersection(const Vec3f &P0, const Vec3f &P1, const Vec3f &planeCenter, const Vec3f &planeNormal, Vec3f* intersection)
{
   Vec3f P10 = P1 - P0;
   Vec3f P20 = planeCenter - P0;
   float nDot10 = planeNormal.dot(P10);
   float nDot20 = planeNormal.dot(P20);

   if (nDot10 == 0){
     return false;
   }

   float u = nDot20/nDot10;

   if (u > 1.0 or u < 0.0) {
     return false;
   }

   *intersection = u*P0 + (1-u)*P1;

   return true;
}

//back 2
bool SliceViewer::linePlaneIntersection(const Vec3f &P0, const Vec3f &P1, const Vec3f &planeCenter, const Vec3f &planeNormal, Vec3f* intersection)
{
   Vec3f P10 = P1 - P0;
   Vec3f P20 = planeCenter - P0;
   float nDot10 = planeNormal.dot(P10);
   float nDot20 = planeNormal.dot(P20);
   Vec3f w = P0 -planeCenter;
   float fac = -planeNormal.dot(w)/nDot10;
   Vec3f t = P10*fac;

   if (nDot10 == 0){
     return false;
   }

   float u = nDot20/nDot10;

   if (u > 1.0 or u < 0.0) {
     return false;
   }

  *intersection = P0 +t;  //testing a new intersect calculation

   return true;
}
*/

bool SliceViewer::linePlaneIntersection(const Vec3f &P0, const Vec3f &P1, const Vec3f &planeCenter, const Vec3f &planeNormal, Vec3f* intersection)
{
   Vec3f P10 = P1 - P0;
   Vec3f P20 = planeCenter - P0;
   Vec3f P02 = P0 - planeCenter;
   float nDot10 = planeNormal.dot(P10);
   float nDot20 = planeNormal.dot(P20);

   if (nDot10 == 0){
     return false;
   }

   float u = nDot20/nDot10;

   if (u > 1.0 || u < 0.0) {
     return false;
   }

  *intersection = P0 +P10*u;  //testing a new intersect calculation

   return true;
}



std::vector<Vec3f> SliceViewer::linspace( Vec3f a, Vec3f b, int n) {
  std::vector<Vec3f> arr;
  Vec3f ba = b-a;
  Vec3f step = Vec3f(ba.x/(n-1.0),ba.y/(n-1.0),ba.z/(n-1.0));
//  std::cout << ba.x << " " << ba.y << " " << ba.z << "\n";
//  std::cout << step.x << " " << step.y << " " << step.z << "\n";

  for (int i = 0; i < n; i++) {
    arr.push_back(a);
    a += step;
  }
  return arr;
}

Vec3f SliceViewer::point2Dto3D(Vec3f Q, Vec3f H, Vec3f K, float u, float v){
  return Vec3f(Q.x+u*H.x+v*K.x,Q.y+u*H.y+v*K.y,Q.z+u*H.z+v*K.z);
}

bool SliceViewer::parallelLinespace(Vec3f p0, Vec3f p1, Vec3f p2, Vec3f p3, std::vector<Vec3f> &list, std::vector<Vec3f> &list2, float aDirection, float oDirection, std::vector<Vec3f> &points){
  Vec3f a = p0-p1;
  Vec3f b = p2-p3;
  float t = a.dot(b)/(a.mag()*b.mag());
  t = round(t*10000)/10000;
  int n = aDirection;
  if ((p0-p1).mag()/(p0-p2).mag() == aDirection/oDirection){ 
    n = oDirection; 
  }
  std::cout.flush();
  std::cout << "t = " << t << "\n";
  if (t == -1.0){
    list = linspace(p0,p1,n);
    list2 = linspace(p3,p2,n);
    points.push_back(p0);
    points.push_back(p1);
    points.push_back(p3);
    points.push_back(p2);
    return true;
  } else if (t == 1.0){
    list = linspace(p0,p1,n);
    list2 = linspace(p2,p3,n);
    points.push_back(p0);
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    return true;
  } 
  return false;
}

//does this needs to be in voxels?  currently it isn't
Array SliceViewer::slice(Vec3f planeCenter, Vec3f planeNormal, std::vector<Vec3f> &finalPointList){
 //assume point and vector are given in cell*width, cell*height, cell*depth coordinate system
 //point and vector define a plane
 // nice page on cube plane intersection
 // http://cococubed.asu.edu/code_pages/raybox.shtml
 // calculate Maxs
  float xMax =  m_array->width()* m_voxWidth[0];
  float yMax =  m_array->height()* m_voxWidth[1];
  float zMax =  m_array->depth()* m_voxWidth[2];

  std::cout << "values " << xMax << " " << yMax << " " << zMax << "\n";
//
//calculate intersections 
  std::vector<Vec3f> P;
  Vec3f intersection;
  if (linePlaneIntersection(Vec3f(0,0,0),Vec3f(0,0,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(0,yMax,0),Vec3f(0,yMax,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(xMax,yMax,0),Vec3f(xMax,yMax,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(xMax,0,0),Vec3f(xMax,0,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(0,0,0),Vec3f(xMax,0,0),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(0,yMax,0),Vec3f(xMax,yMax,0),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(0,yMax,zMax),Vec3f(xMax,yMax,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(xMax,0,zMax),Vec3f(0,0,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(0,0,0),Vec3f(0,yMax,0),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(0,yMax,zMax),Vec3f(0,0,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(xMax,0,zMax),Vec3f(xMax,yMax,zMax),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);
  if (linePlaneIntersection(Vec3f(xMax,0,0),Vec3f(xMax,yMax,0),planeCenter,planeNormal,&intersection))
    P.push_back(intersection);

  Array result = Array();// XXX
  
  if (P.size() > 1){
    Vec2f *p2D = new Vec2f[P.size()];
    p2D[0] = Vec2f(0,0);
    float x = sqrt(pow(P[1].x-P[0].x,2.0)+pow(P[1].y-P[0].y,2.0)+pow(P[1].z-P[0].z,2.0));
    std::cout << x << "\n";
    p2D[1] = Vec2f(x,0);
    if(P.size() == 2){
      //super easy, it's just a line :)
      x = ceil(x);
      result.format(1, m_array->type(), x);
      std::vector<Vec3f> space = linspace(P[0], P[1], x);
      for (unsigned j = 0; j < space.size(); j++){
        Vec3f point = space[j];
        float temp[1] = {0};
        if (point.x >= 0 && point.y >= 0 && point.z >= 0 && point.x <=  m_array->width()* m_voxWidth[2] && point.y <=  m_array->height()* m_voxWidth[1] && point.z <=  m_array->depth()* m_voxWidth[2]){
          Vec3f p = Vec3f(point.x/ m_voxWidth[0],point.y/ m_voxWidth[1],point.z/ m_voxWidth[2]);
          m_array->read_interp(temp, p);
        }
        result.write(temp,j);
      }
    } else {
      //Oh noes!  It's more than a line, this is a little more complex
      float minA2D = 0, minO2D = 0, maxA2D = 0, maxO2D = 0;
      //what are all these points in 2d on a plane
      //http://stackoverflow.com/questions/10702099/computing-two-vectors-that-are-perpendicular-to-third-vector-in-3d
      Vec3f y_axis = planeNormal.cross((P[0]-planeCenter).normalize());
      Vec3f z_axis = planeNormal.cross(y_axis);
      //http://stackoverflow.com/questions/23472048/projecting-3d-points-to-2d-plane
      for (unsigned i = 0; i < P.size(); i++) {
        Vec3f pi = P[i]-planeCenter;
        float t_1 = y_axis.dot(pi);
        float t_2 = z_axis.dot(pi);
        minA2D = std::min(minA2D, t_1);
        maxA2D = std::max(maxA2D, t_1);
        minO2D = std::min(minO2D, t_2);
        maxO2D = std::max(maxO2D, t_2);
        p2D[i] = Vec2f(t_1, t_2);
      }
      int aDirection = ceil(maxA2D - minA2D);
      int oDirection = ceil(maxO2D - minO2D);
      //http://math.stackexchange.com/questions/525829/how-to-find-the-3d-coordinate-of-a-2d-point-on-a-known-plane
      result.format(1, m_array->type(), aDirection, oDirection);
      Vec3f p0 = point2Dto3D(planeCenter,y_axis,z_axis,minA2D,minO2D);
      Vec3f p1 = point2Dto3D(planeCenter,y_axis,z_axis,minA2D,maxO2D);
      Vec3f p2 = point2Dto3D(planeCenter,y_axis,z_axis,maxA2D,maxO2D);
      Vec3f p3 = point2Dto3D(planeCenter,y_axis,z_axis,maxA2D,minO2D);
      std::cout << "p0 :" << p0.x << " " << p0.y << " " << p0.z << "\n";
      std::cout << "p1 :" << p1.x << " " << p1.y << " " << p1.z << "\n";
      std::cout << "p2 :" << p2.x << " " << p2.y << " " << p2.z << "\n";
      std::cout << "p3 :" << p3.x << " " << p3.y << " " << p3.z << "\n";
      //Check to see if two lines intersect
      std::vector<Vec3f> list;
      std::vector<Vec3f> list2;
      if (!parallelLinespace(p0, p1, p2, p3, list, list2, maxA2D-minA2D, maxO2D-minO2D, finalPointList)){
        if (!parallelLinespace(p0, p2, p1, p3, list, list2, maxA2D-minA2D, maxO2D-minO2D, finalPointList)){
          parallelLinespace(p0, p3, p1, p2, list, list2, maxA2D-minA2D, maxO2D-minO2D, finalPointList);
        }
      }
      std::cout << "finalPointList Length:" << finalPointList.size() << "\n";
      //now lets fill the results
      for (unsigned i = 0; i < list.size(); i++){
        std::vector<Vec3f> space = linspace(list[i], list2[i], oDirection);  //XXX should this be oDirection or aDirection, please check!
        for (unsigned j = 0; j < space.size(); j++){
          Vec3f point = space[j];
          float temp[1] = {0};
          if (point.x >= 0 && point.y >= 0 && point.z >= 0 && point.x <=  m_array->width()* m_voxWidth[2] && point.y <=  m_array->height()* m_voxWidth[1] && point.z <=  m_array->depth()* m_voxWidth[2]){
            Vec3f p = Vec3f(point.x/ m_voxWidth[0],point.y/ m_voxWidth[1],point.z/ m_voxWidth[2]);
            m_array->read_interp(temp, p);
          }
          result.write(temp,i,j);
        }
      }
    }
   delete [] p2D; 
  }
  else if (P.size() == 1) {
    //Intersects at one point, this is super easy!
    //calculate point and return array with single point
    result.format(1, m_array->type(),1);
    Vec3f point = P[0];
    float temp[1] = {0};
    if (point.x >= 0 && point.y >= 0 && point.z >= 0 && point.x <=  m_array->width()* m_voxWidth[2] && point.y <=  m_array->height()* m_voxWidth[1] && point.z <=  m_array->depth()* m_voxWidth[2]){
      Vec3f p = Vec3f(point.x/ m_voxWidth[0],point.y/ m_voxWidth[1],point.z/ m_voxWidth[2]);
      m_array->read_interp(temp, p);
    }
    result.write(temp,0);
  }
//  for (int i = 0; i < finalPointList.size(); i++){
//    finalPointList[i] = finalPointList[i]/Vec3f(xMax,yMax,zMax);
//  }
//  std::cout << "wtf" << "\n";
  return result;
}


} // al::
