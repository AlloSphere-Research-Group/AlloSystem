# -*- coding: utf-8 -*-
"""
Created on Wed Mar 30 20:16:52 2016

@author: andres
"""

import appnode
import os

nodes = []

filenames = ["src/%s.cpp"%name for name in ["simulator", "graphics", "control"] ]

node = appnode.BuildNode('local', False)

configuration = {"prebuild_commands" : '',
                 "build_commands" : ['$$cmake', '$$make'],
                 "project_src" : filenames
                }
node.configure(**configuration)
nodes.append(node)


