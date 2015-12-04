#ifndef INCLUDE_AL_SHADERMANAGER_HPP
#define INCLUDE_AL_SHADERMANAGER_HPP

#include "alloutil/al_ResourceManager.hpp"
#include <map>

using namespace std;

struct ShaderManager {
  std::map<std::string, ShaderProgram*> shaderMap;
  std::string vertLibCode;
  std::string fragLibCode;
  ResourceManager rm;

  ShaderProgram* get(std::string name) {
    return shaderMap[name];
  }

  void addShaderString(std::string name, std::string vertCode, std::string fragCode) {
    // destroy shader if one already exists with same name
    if(shaderMap.count(name)){
      shaderMap[name]->destroy();
    }

    Shader vert, frag;
    vert.source(vertLibCode + vertCode, Shader::VERTEX).compile();
    vert.printLog();
    frag.source(fragLibCode + fragCode, Shader::FRAGMENT).compile();
    frag.printLog();

    ShaderProgram *s = new ShaderProgram();
    s->attach(vert).attach(frag).link();
    s->printLog();
    //s->listParams();

    shaderMap[name] = s;
  }

  void addShaderFile(std::string pName, std::string vName, std::string fName) {
    // destroy shader if one already exists with same name
    if(shaderMap.count(pName)){
      shaderMap[pName]->destroy();
    }
    
    rm.paths.addSearchPath("../../", true);
    rm.paths.addAppPaths();
    rm.paths.addSearchPath(".", true);
    
    if(rm.add(vName, true))
      std::cout << "added " << vName << std::endl;
    else
      std::cout << "couldn't add " << vName << std::endl;
    if(rm.add(fName, true))
      std::cout << "added " << fName << std::endl;
    else
      std::cout << "couldn't add " << fName << std::endl;

    Shader vert, frag;

    // std::cout << vname << std::endl << rm.data(vname) << std::endl;
    // std::cout << fname << std::endl << rm.data(fname) << std::endl;

    if(rm[vName].loaded) {
      std::cout << "vertex loaded\n" << std::endl;
      std::string vsrc = rm.data(vName);
      std::cout << vsrc << std::endl;

      vert.source(vsrc, Shader::VERTEX);

      std::cout << "source complete" << std::endl;

      // std::cout << vname << std::endl << rm.data(vname) << std::endl;
      vert.compile();
      // vert.source(rm.data(vname), Shader::VERTEX).compile().printLog();
      std::cout << "vertex compiled" << std::endl;
    }
    if(rm[fName].loaded) frag.source(rm.data(fName), Shader::FRAGMENT).compile().printLog();
    
    std::cout << "test" << std::endl;

    ShaderProgram *s = new ShaderProgram();

    std::cout << "test2" << std::endl;

    std::cout << "Attaching Vertex Shader: " << vName << std::endl;
    std::cout << "Attaching Fragment Shader: " << fName << std::endl;
    
    if(rm[vName].loaded) s->attach(vert);
    if(rm[fName].loaded) s->attach(frag);

    std::cout << "test3" << std::endl;

    s->link(); 
    s->printLog();
    // s->listParams();

    shaderMap[pName] = s;
  }

  // void addShaderFile(std::string pName, std::string vName, std::string fName) {
  //   // destroy shader if one already exists with same name
  //   if(shaders.count(pName)){
  //     shaders[pName]->destroy();
  //   }
    
  //   rm.paths.addSearchPath("../../", true);
  //   rm.paths.addAppPaths();
  //   rm.paths.addSearchPath(".", true);
    
  //   std::string vname = name + ".vert";
  //   std::string fname = name + ".frag";
  //   std::string gname = name + ".geom";
    
  //   if(rm.add(vname, true))
  //     std::cout << "added " << vname << std::endl;
  //   else
  //     std::cout << "couldn't add " << vname << std::endl;
  //   if(rm.add(fname, true))
  //     std::cout << "added " << fname << std::endl;
  //   else
  //     std::cout << "couldn't add " << fname << std::endl;

  //   if(hasGeo)
  //     rm.add(gname, true);
    
  //   Shader vert2, frag, geom;

  //   // std::cout << vname << std::endl << rm.data(vname) << std::endl;
  //   // std::cout << fname << std::endl << rm.data(fname) << std::endl;

  //   if(rm[vname].loaded) {
  //     std::cout << "vertex loaded\n" << std::endl;
  //     std::string vsrc = rm.data(vname);
  //     std::cout << vsrc << std::endl;

  //     vert2.source(vsrc, Shader::VERTEX);

  //     std::cout << "source complete" << std::endl;

  //     // std::cout << vname << std::endl << rm.data(vname) << std::endl;
  //     vert2.compile();
  //     // vert.source(rm.data(vname), Shader::VERTEX).compile().printLog();
  //     std::cout << "vertex compiled" << std::endl;
  //   }
  //   if(rm[fname].loaded) frag.source(rm.data(fname), Shader::FRAGMENT).compile().printLog();
  //   if(hasGeo)
  //     if(rm[gname].loaded) geom.source(rm.data(gname), Shader::GEOMETRY).compile().printLog();

  //   std::cout << "test" << std::endl;

  //   ShaderProgram *s = new ShaderProgram();

  //   std::cout << "test2" << std::endl;

  //   std::cout << "Attaching Vertex Shader: " << vname << std::endl;
  //   std::cout << "Attaching Fragment Shader: " << fname << std::endl;
    
  //   if(rm[vname].loaded) s->attach(vert2);
  //   if(rm[fname].loaded) s->attach(frag);

  //   std::cout << "test3" << std::endl;

  //   if(hasGeo)
  //     if(rm[gname].loaded) {
  //       std::cout << "Attaching Geometry Shader: " << gname << std::endl;
  //       s->setGeometryInputPrimitive(Graphics::LINES);
  //       s->setGeometryOutputPrimitive(Graphics::TRIANGLE_STRIP);
  //       s->setGeometryOutputVertices(18);
  //       s->attach(geom);
  //     }

  //   s->link(); 
  //   s->printLog();

  //   shaders[name] = s;
  // }
  
  // checks modified time tag and returns true if files in resource manager had been changed
  bool poll() {
    return rm.poll();
  }
};

#endif
