/*
 * al_ShaderManager.hpp
 *
 * Manages shader programs loaded from files, and reloads them as shader files are changed.
 *
 * File author(s):
 *  Tim Wood, 2015, fishuyo@gmail.com
 *  Kon Hyong Kim, 2015, konhyong@gmail.com
 */

#ifndef __ShaderManager_HPP__
#define __ShaderManager_HPP__

#include "alloutil/al_ResourceManager.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include <map>

namespace al {
struct Shaders {
  static std::map< std::string, ShaderProgram* > shaders;
  static std::string vertLibCode;
  static std::string fragLibCode;
  static al::ResourceManager rm;
  
  static SearchPaths& paths(){ return rm.paths;}

  // get a pointer to a loaded shader program with name
  static ShaderProgram* get(std::string name){
    return shaders[name];
  }

  // load shader from code
  static void loadCode(std::string name, std::string vertCode, std::string fragCode){
    // destroy shader if one already exists with same name
    if(shaders.count(name)){
      shaders[name]->destroy();
    }

    Shader vert, frag;
    vert.source(vertLibCode + vertCode, Shader::VERTEX).compile();
    vert.printLog();
    frag.source(fragLibCode + fragCode, Shader::FRAGMENT).compile();
    frag.printLog();

    ShaderProgram *s = new ShaderProgram();
    s->attach(vert).attach(frag).link();
    s->printLog();

    shaders[name] = s;
  }

  // load shader from files
  // set hasGeo as true if it has geometry shaders
  static void load(std::string name, bool hasGeo=false){
    std::string vname = name + ".vert";
    std::string fname = name + ".frag";
    std::string gname = "";
    if(hasGeo) gname = name + ".geom"; 
    load(name, vname, fname, gname);
  }
  // load shader from files
  // set hasGeo as true if it has geometry shaders
  static void load(std::string name, std::string vname, std::string fname, std::string gname=""){
    // destroy shader if one already exists with same name
    if(shaders.count(name)){
      shaders[name]->destroy();
    }
    
    // rm.paths.addAppPaths();
    
    rm.add(vname, true);
    rm.add(fname, true);
    
    if(gname != "")
      rm.add(gname, true);
    
    Shader vert, frag, geom;

    if(rm[vname].loaded) vert.source(vertLibCode + rm.data(vname), Shader::VERTEX).compile().printLog();
    if(rm[fname].loaded) frag.source(fragLibCode + rm.data(fname), Shader::FRAGMENT).compile().printLog();
    if(gname != "")
      if(rm[gname].loaded) geom.source(rm.data(gname), Shader::GEOMETRY).compile().printLog();

    ShaderProgram *s = new ShaderProgram();

    if(rm[vname].loaded) s->attach(vert);
    if(rm[fname].loaded) s->attach(frag);

    if(gname != "")
      if(rm[gname].loaded) {
        std::cout << "Attaching Geometry Shader: " << gname << std::endl;
        s->setGeometryInputPrimitive(Graphics::LINES);
        s->setGeometryOutputPrimitive(Graphics::TRIANGLE_STRIP);
        s->setGeometryOutputVertices(18);
        s->attach(geom);
      }

    if(rm[vname].loaded){
      s->link(); 
      s->printLog();
    }

    shaders[name] = s;
  }
  
  // checks modified time tag and returns true if files in resource manager had been changed
  static bool poll() {
    return rm.poll();
  }

};

std::map< std::string, ShaderProgram* > Shaders::shaders;
std::string Shaders::vertLibCode;
std::string Shaders::fragLibCode;
al::ResourceManager Shaders::rm;

}

#endif
