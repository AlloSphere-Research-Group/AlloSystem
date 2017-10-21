# -*- coding: utf-8 -*-
"""
Created on Tue Mar 29 18:02:47 2016

@author: andres
"""

import appnode

nodes = []

# ---- GR01 node builds and deploys to the other GR machines
deploy_to = [['gr%02i.1g'%(i+ 2) for i in range(13)]]
node = appnode.RemoteBuildNode('gr01.1g','sphere', deploy_to)
configuration = {"build_commands" : ['$$cmake', '$$make'],
                 "project_src" : ['src/graphics.cpp']
                }
node.configure(**configuration)
nodes.append(node)

# ---- Audio machine build. Simulator
node = appnode.RemoteBuildNode('audio.1g', 'sphere', [['audio.1g']])
configuration = {"build_commands" : ['$$cmake', '$$make'],
                 "project_src" : ['src/simulator.cpp', 'src/audio.cpp'],
                 "cmake" : "/usr/local/bin/cmake",
                 "scratch_path" : "/Users/sphere/scratch"
                }
node.configure(**configuration)
nodes.append(node)

# ---- Build control interface if present
node = appnode.RemoteBuildNode('control.1g', 'sphere', [['control.1g']])
configuration = {"build_commands" : ['$$cmake', '$$make'],
                 "project_src" : ['src/control.cpp'],
                 "scratch_path" : "/home/sphere/scratch"
                }
node.configure(**configuration)
nodes.append(node)

## ---- Build audio app interface if present
#node = appnode.RemoteBuildNode('audio.1g', 'sphere', [['audio.1g']])
#configuration = {"build_commands" : ['$$cmake', '$$make'],
#                 "project_src" : ['src/audio.cpp'],
#                 "cmake" : "/usr/local/bin/cmake"
#                }
#node.configure(**configuration)
#nodes.append(node)