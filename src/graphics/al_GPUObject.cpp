#include "allocore/graphics/al_GPUObject.hpp"

#include <map>
#include <set>

using namespace al;

typedef std::set<al::GPUObject *> ResourceSet;
typedef std::map<int, ResourceSet> ContextMap;
typedef std::map<al::GPUObject *, int> ResourceMap;

// Global data structures for managing context resources
static ContextMap g_context_resources;
static ResourceMap g_resources;
static int g_next_context_id = 0;

GPUContext :: GPUContext() {
	mContextID = g_next_context_id;
	g_next_context_id++;
}

void GPUContext :: contextDestroy() {
	ContextMap::iterator it = g_context_resources.find(mContextID);
	if(it != g_context_resources.end()) {
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
	
	ContextMap::iterator it = g_context_resources.find(ctx);
	if(it == g_context_resources.end()) {
		g_context_resources.insert(std::pair<int, ResourceSet>(ctx, ResourceSet()));
		it = g_context_resources.find(ctx);
	}
	it->second.insert(this);
	g_resources.insert(std::pair<al::GPUObject *, int>(this, ctx));
}

void GPUObject :: contextUnregister() {
	ResourceMap::iterator rit = g_resources.find(this);
	if(rit != g_resources.end()) {
		
		ContextMap::iterator it = g_context_resources.find( rit->second );
		if(it != g_context_resources.end()) {
			ResourceSet &ctx_set = it->second;
			ResourceSet::iterator sit = ctx_set.find(this);
			if(sit != ctx_set.end()) {
				ctx_set.erase(sit);
			}
		}
		
		g_resources.erase(rit);
	}
}


