#ifndef INCLUDE_AL_SHADERMANAGER_HPP
#define INCLUDE_AL_SHADERMANAGER_HPP

#include "allocore/graphics/al_Shader.hpp"
#include "alloutil/al_ResourceManager.hpp"
#include <map>

struct ShaderManager {
  std::map<std::string, al::ShaderProgram*> shaderMap;
  std::string vertLibCode;
  std::string fragLibCode;
  al::ResourceManager rm;

  al::ShaderProgram* get(std::string name) {
    return shaderMap[name];
  }

  al::ShaderProgram* addShaderString(std::string name, std::string vertCode, std::string fragCode) {
    // destroy shader if one already exists with same name
    if(shaderMap.count(name)){
      shaderMap[name]->destroy();
    }

    al::Shader vert, frag;
    vert.source(vertLibCode + vertCode, al::Shader::VERTEX).compile();
    vert.printLog();
    frag.source(fragLibCode + fragCode, al::Shader::FRAGMENT).compile();
    frag.printLog();

    al::ShaderProgram *s = new al::ShaderProgram();
    s->attach(vert).attach(frag).link();
    s->printLog();
    //s->listParams();

    shaderMap[name] = s;

    return s;
  }

  al::ShaderProgram* addShaderFile(std::string pName, std::string vName, std::string fName) {
    // destroy shader if one already exists with same name
    if(shaderMap.count(pName)){
      shaderMap[pName]->destroy();
    }
    
    rm.paths.addSearchPath("../../", true);
    rm.paths.addAppPaths();
    rm.paths.addSearchPath(".", true);
    
    rm.add(vName);
    rm.add(fName);

    al::Shader vert, frag;

    if(rm[vName].loaded) vert.source(vertLibCode + rm.data(vName), al::Shader::VERTEX).compile().printLog();
    if(rm[fName].loaded) frag.source(fragLibCode + rm.data(fName), al::Shader::FRAGMENT).compile().printLog();

    al::ShaderProgram *s = new al::ShaderProgram();

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

  al::ShaderProgram* addShaderFile(std::string pName) {
    std::string vName = pName + ".vert";
    std::string fName = pName + ".frag";

    return addShaderFile(pName, vName, fName);
  }

  al::ShaderProgram* addShaderFile(std::string pName, std::string vName, std::string gName, std::string fName) {
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
    
    al::Shader vert, frag, geom;

    if(rm[vName].loaded) vert.source(vertLibCode + rm.data(vName), al::Shader::VERTEX).compile().printLog();
    if(rm[gName].loaded) geom.source(fragLibCode + rm.data(gName), al::Shader::GEOMETRY).compile().printLog();
    if(rm[fName].loaded) frag.source(rm.data(fName), al::Shader::FRAGMENT).compile().printLog();

    al::ShaderProgram *s = new al::ShaderProgram();

    std::cout << "Attaching Vertex Shader: " << vName << std::endl;
    if(rm[vName].loaded) s->attach(vert);
    
    if(rm[gName].loaded) {
      std::cout << "Attaching Geometry Shader: " << gName << std::endl;
      s->setGeometryInputPrimitive(al::Graphics::LINES);
      s->setGeometryOutputPrimitive(al::Graphics::TRIANGLE_STRIP);
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
    for (std::map<std::string, al::ShaderProgram*>::iterator it = shaderMap.begin(); it != shaderMap.end(); ++it) {
      (it->second)->invalidate();
    }
  }
};

#endif
