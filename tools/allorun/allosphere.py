# -*- coding: utf-8 -*-
"""
Created on Tue Mar 29 18:02:47 2016

@author: andres
"""

import appnode

nodes = []

# GR01 node builds and deploys to the other GR machines
node = appnode.BuildNode('gr01')
nodes.append(node)

# Audio machine builds 
node = appnode.BuildNode('audio')
nodes.append(node)

