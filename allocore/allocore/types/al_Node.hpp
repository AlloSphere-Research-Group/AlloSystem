#ifndef INCLUDE_AL_NODE_HPP
#define INCLUDE_AL_NODE_HPP

/*	Allocore --
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
	Node class for creating a tree data structure.

	File author(s):
	Lance Putnam, 2021, putnam.lance@gmail.com
*/

#include <functional>

namespace al{

/// Tree node with links to parent, child and sibling
template <class Node>
class TreeNode {
public:

	/// Add child to this node

	/// If the node to be added already has a parent it will be detached from 
	/// its existing parent. The node will not be added if it is already a child
	/// of this node.
	Node& addChild(Node& n){
		if(n.mParent != self()){
			n.removeFromParent();
			n.mParent = self();

			// insert front
			//if(mChild) lastChild().mSibling = &n;
			//else mChild = &n;

			// insert back (faster)
			if(mChild) n.mSibling = mChild;
			mChild = &n;
		}
		return *self();
	}

	/// Remove from parent while retaining descendants
	Node& removeFromParent(){
		if(mParent && mParent->mChild){
			// re-patch parent's child?
			if(mParent->mChild == self()){
				mParent->mChild = mSibling;
			}
			// re-patch the sibling chain?
			else{
				// I must be one of parent->child's siblings
				auto * temp = mParent->mChild;
				while(temp->mSibling){
					if(temp->mSibling == self()) {
						temp->mSibling = mSibling; 
						break;
					}
					temp = temp->mSibling;
				}
			}
			// no more parent or sibling, but child is still valid
			mParent = mSibling = nullptr;
		}
		return *self();
	}

	/// Perform depth-first traversal

	/// Walks the tree starting at this node and visiting children first, then 
	/// siblings of all descendants. The siblings of this node are not visited.
	void traverseDepth(const std::function<void(Node&, int depth)>& onNode){
		auto * const root = self();
		auto * n = root;
		int depth = 0;

		// Early exit if node has no children
		if(n->isLeaf()){
			onNode(*n, depth);
			return;
		}

		// Only valid if node has children
		while(n){
			onNode(*n, depth);
			if(n->mChild){			/* Down to child */
				++depth;
				n = n->mChild;
			}
			else if(n->mSibling){	/* Across to sibling */
				n = n->mSibling;
			}
			else{					/* Up and over to next branch */
				if(root == n || !n->mParent) return; /* Triggers if only node in tree */
				while(n->mParent){
					--depth;
					n = n->mParent;
					if(root == n) return;
					else if(n->mSibling){ n = n->mSibling; break; }
				}
			}
		}
	}

	void traverse(const std::function<void(Node&)>& onNode){
		traverseDepth([&onNode](Node& n, int){ onNode(n); });
	}

	bool isLeaf() const { return mChild == nullptr; }

	bool hasParent(const Node& n) const { return mParent == &n; }
	bool hasParent() const { return mParent != nullptr; }
	const Node& parent() const { return *mParent; }
	Node& parent(){ return *mParent; }

	bool hasSibling() const { return mSibling != nullptr; }
	const Node& sibling() const { return *mSibling; }
	Node& sibling(){ return *mSibling; }

	bool hasChild() const { return mChild != nullptr; }
	const Node& child() const { return *mChild; }
	Node& child(){ return *mChild; }

private:
	Node * mParent = nullptr;
	Node * mSibling = nullptr;
	Node * mChild = nullptr;
	Node * self(){ return static_cast<Node*>(this); }
	const Node * self() const { return static_cast<Node*>(this); }

	// Get last child (call only if mChild)
	Node& lastChild(){
		auto * n = mChild;
		while(n->mSibling) n = n->mSibling;
		return *n;
	}
};

} // al::

#endif
