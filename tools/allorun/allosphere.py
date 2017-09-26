# -*- coding: utf-8 -*-
"""
Created on Tue Mar 29 18:02:47 2016

@author: andres
"""

import appnode

nodes = []

# GR01 node builds and deploys to the other GR machines
deploy_to = [['gr%02i'%(i+ 1) for i in range(13)], ['control']]
node = appnode.RemoteBuildNode('gr01','sphere', deploy_to)
configuration = {"build_commands" : ['$$cmake', '$$make'],
                 "project_src" : ['src/graphics.cpp', 'src/control.cpp']
                }
node.configure(**configuration)
nodes.append(node)

# Audio machine builds
node = appnode.RemoteBuildNode('audio', 'sphere', ['audio'])
configuration = {"build_commands" : ['$$cmake', '$$make'],
                 "project_src" : ['src/simulator.cpp'],
                 "cmake" : "/usr/local/bin/cmake"
                }
node.configure(**configuration)
nodes.append(node)

