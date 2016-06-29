# -*- coding: utf-8 -*-
"""
Created on Tue Mar 29 18:02:47 2016

@author: andres
"""

import appnode

nodes = []

# GR01 node builds and deploys to the other GR machines
deploy_to = ['gr%02i'%(i+ 1) for i in range(13)]
node = appnode.RemoteBuildNode('gr01', '','sphere', deploy_to)
nodes.append(node)

# Audio machine builds 
node = appnode.BuildNode('audio')
nodes.append(node)

