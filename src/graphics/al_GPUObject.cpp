#include "allocore/graphics/al_GPUObject.hpp"

#include <map>
#include <set>

using namespace al;

typedef std::set<al::GPUObject *> ResourceSet;
typedef std::map<int, ResourceSet> ContextMap;
typedef std::map<al::GPUObject *, int> ResourceMap;


ContextMap * getContextMap() {
	static ContextMap * instance = new ContextMap;
	return instance;
}

ResourceMap * getResourceMap() {
	static ResourceMap * instance = new ResourceMap;
	return instance;
}

int getNextContextID() {
	static int g_next_context_id = 0;
	int result = g_next_context_id;
	g_next_context_id++;
	return result;
}

GPUContext :: GPUContext() {
	mContextID = getNextContextID();
}

void GPUContext :: contextDestroy() {
	ContextMap& contexts = *getContextMap();
	ContextMap::iterator it = contexts.find(mContextID);
	if(it != contexts.end()) {
		ResourceSet &ctx_set = it->second;
		ResourceSet::iterator sit = ctx_set.begin();
		ResourceSet::iterator site = ctx_set.end();
		for(; sit != site; ++sit) {
			(*sit)->destroy();
		}
	}
}

void GPUObject :: contextRegister(int ctx) {
	contextUnregister();
	ContextMap& contexts = *getContextMap();
	ResourceMap& resources = *getResourceMap();
	contexts[ctx].insert(this);
	resources[this] = ctx;
}

void GPUObject :: contextUnregister() {
	ContextMap& contexts = *getContextMap();
	ResourceMap& resources = *getResourceMap();
	
	ResourceMap::iterator rit = resources.find(this);
	if(rit != resources.end()) {
		ContextMap::iterator it = contexts.find( rit->second );
		if(it != contexts.end()) {
			ResourceSet &ctx_set = it->second;
			ResourceSet::iterator sit = ctx_set.find(this);
			if(sit != ctx_set.end()) {
				ctx_set.erase(sit);
			}
		}
		resources.erase(rit);
	}
}


