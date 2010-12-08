#include "allocore/graphics/al_GPUObject.hpp"

#include <string>
#include <map>
#include <set>

using namespace al;

typedef std::set<al::GPUObject *> ResourceSet;
typedef std::map<std::string, ResourceSet> ContextMap;
typedef std::map<al::GPUObject *, std::string> ResourceMap;

// Global data structures for managing context resources
ContextMap g_context_resources;
ResourceMap g_resources;



void GPUContext :: contextDestroy() {
	std::string ctx(contextName());
	ContextMap::iterator it = g_context_resources.find(ctx);
	if(it != g_context_resources.end()) {
		ResourceSet &ctx_set = it->second;
		ResourceSet::iterator sit = ctx_set.begin();
		ResourceSet::iterator site = ctx_set.end();
		for(; sit != site; ++sit) {
			(*sit)->destroy();
		}
	}
}

void GPUObject :: contextRegister(std::string ctx) {
	ContextMap::iterator it = g_context_resources.find(ctx);
	if(it == g_context_resources.end()) {
		g_context_resources.insert(std::pair<std::string, ResourceSet>(ctx, ResourceSet()));
		it = g_context_resources.find(ctx);
	}
	it->second.insert(this);
	g_resources.insert(std::pair<al::GPUObject *, std::string>(this, ctx));
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


