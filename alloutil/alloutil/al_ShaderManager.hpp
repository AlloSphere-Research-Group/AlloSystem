#ifndef INCLUDE_AL_SHADERMANAGER_HPP
#define INCLUDE_AL_SHADERMANAGER_HPP

/*  Allocore --
  Multimedia / virtual environment application class library

  Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
  Copyright (C) 2012. The Regents of the University of California.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    Neither the name of the University of California nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.


  File description:
  Utility for loading & watching shader files

  File author(s):
  Kon Hyong Kim, 2015, konhyong@gmail.com
*/
  
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
