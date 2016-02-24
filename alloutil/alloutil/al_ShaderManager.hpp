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

  ShaderProgram* addShaderString(std::string name, std::string vertCode, std::string fragCode) {
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

    return s;
  }

  ShaderProgram* addShaderFile(std::string pName, std::string vName, std::string fName) {
    // destroy shader if one already exists with same name
    if(shaderMap.count(pName)){
      shaderMap[pName]->destroy();
    }
    
    rm.paths.addSearchPath("../../", true);
    rm.paths.addAppPaths();
    rm.paths.addSearchPath(".", true);
    
    rm.add(vName);
    rm.add(fName);

    Shader vert, frag;

    if(rm[vName].loaded) vert.source(vertLibCode + rm.data(vName), Shader::VERTEX).compile().printLog();
    if(rm[fName].loaded) frag.source(fragLibCode + rm.data(fName), Shader::FRAGMENT).compile().printLog();

    ShaderProgram *s = new ShaderProgram();

    std::cout << "Attaching Vertex Shader: " << vName << std::endl;
    std::cout << "Attaching Fragment Shader: " << fName << std::endl;
    
    if(rm[vName].loaded) s->attach(vert);
    if(rm[fName].loaded) s->attach(frag);

    s->link(); 
    s->printLog();
    // s->listParams();

    shaderMap[pName] = s;

    return s;
  }

  ShaderProgram* addShaderFile(std::string pName) {
    std::string vName = pName + ".vert";
    std::string fName = pName + ".frag";

    return addShaderFile(pName, vName, fName);
  }

  ShaderProgram* addShaderFile(std::string pName, std::string vName, std::string gName, std::string fName) {
    // destroy shader if one already exists with same name
    if(shaderMap.count(pName)){
      shaderMap[pName]->destroy();
    }
    
    rm.paths.addSearchPath("../../", true);
    rm.paths.addAppPaths();
    rm.paths.addSearchPath(".", true);
    
    rm.add(vName);
    rm.add(fName);
    rm.add(gName);
    
    Shader vert, frag, geom;

    if(rm[vName].loaded) vert.source(vertLibCode + rm.data(vName), Shader::VERTEX).compile().printLog();
    if(rm[gName].loaded) geom.source(fragLibCode + rm.data(gName), Shader::GEOMETRY).compile().printLog();
    if(rm[fName].loaded) frag.source(rm.data(fName), Shader::FRAGMENT).compile().printLog();

    ShaderProgram *s = new ShaderProgram();

    std::cout << "Attaching Vertex Shader: " << vName << std::endl;
    if(rm[vName].loaded) s->attach(vert);
    
    if(rm[gName].loaded) {
      std::cout << "Attaching Geometry Shader: " << gName << std::endl;
      s->setGeometryInputPrimitive(Graphics::LINES);
      s->setGeometryOutputPrimitive(Graphics::TRIANGLE_STRIP);
      s->setGeometryOutputVertices(18);
      s->attach(geom);
    }

    std::cout << "Attaching Fragment Shader: " << fName << std::endl;
    if(rm[fName].loaded) s->attach(frag);

    s->link(); 
    s->printLog();
    // s->listParams();

    shaderMap[pName] = s;

    return s;
  }
  
  // checks modified time tag and returns true if files in resource manager had been changed
  bool poll() {
    return rm.poll();
  }

  void destroy() {
    for (std::map<std::string, ShaderProgram*>::iterator it = shaderMap.begin(); it != shaderMap.end(); ++it) {
      (it->second)->invalidate();
    }
  }
};

#endif
