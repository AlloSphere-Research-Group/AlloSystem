#include "allocore/graphics/al_GPUObject.hpp"

#include <map>
#include <set>

using namespace al;

typedef std::set<al::GPUObject *>		ResourceSet;	// resource list
typedef std::map<int, ResourceSet>		ContextMap;		// context ID to resource list
typedef std::map<al::GPUObject *, int>	ResourceMap;	//
typedef std::map<int, al::GPUContext *>	Contexts;		// context ID to context object

ContextMap& getContextMap() {
	static ContextMap * instance = new ContextMap;
	return *instance;
}

ResourceMap& getResourceMap() {
	static ResourceMap * instance = new ResourceMap;
	return *instance;
}

int getNextContextID() {
	static int g_next_context_id = 0;
	int result = g_next_context_id;
	++g_next_context_id;
	//printf("GPUContext: created new context id %d\n", result);
	return result;
}

Contexts& getContexts(){
	static Contexts * r = new Contexts;
	return *r;
}


GPUContext :: GPUContext() {
	mContextID = getNextContextID();
	getContexts()[contextID()] = this;
}

GPUContext :: ~GPUContext(){
	Contexts& C = getContexts();
	Contexts::iterator it = C.find(contextID());
	if(it != C.end()) C.erase(it);
}

int GPUContext::defaultContextID(){ return 0; }

/*
Scenario:
a. GPUObjects constructed with default context (= 0)
b. Window 1 constructed; assigned context id 0
c. Window 2 constructed; assigned context id 1

GPUObjects need to be handled by Window 2

Problem:
Do we change context id of GPUObjects to 1 or change Window 2 context id to 0?

*/

void GPUContext::makeDefaultContext(){
	const int myID = contextID();
	const int dfID = defaultContextID();
	//printf("%d %d\n", myID, dfID);
	
//	Contexts::iterator it = getContexts().begin();
//	while(it != getContexts().end()){
//		printf("%d\n", it->first);
//		++it;
//	}

	if(myID != dfID){
		Contexts& C = getContexts();
		Contexts::iterator it = C.find(dfID);
		
		// If someone else is already default, then swap IDs with them
		if(it != C.end()){

			// TODO: do we need to migrate existing GPUObjects?
//			ContextMap& id2Objs = getContextMap();
//			
//			ResourceSet dfObjs(id2Objs[dfID]);
//			ResourceSet myObjs(id2Objs[myID]);
//			
//			// swap context IDs of existing GPU objects
//			{
//				//ResourceSet objs = id2Objs[dfID];
//				ResourceSet::iterator i = dfObjs.begin();
//				for(; i != dfObjs.end(); ++i){
//					printf("%p: %d -> %d\n", *i, dfID, myID);
//					(*i)->contextRegister(myID);
//				}
//			}{
//				//ResourceSet objs = id2Objs[myID];
//				ResourceSet::iterator i = myObjs.begin();
//				for(; i != myObjs.end(); ++i){
//					printf("%p: %d -> %d\n", *i, myID, dfID);
//					(*i)->contextRegister(dfID);
//				}
//			}
			
			it->second->mContextID = myID;
			C[myID] = it->second;
		}

		mContextID = dfID;
		C[dfID] = this;
	}
}

void GPUContext :: contextDestroy() { //printf("GPUContext::contextDestroy %d\n", mContextID);
	ContextMap& contexts = getContextMap();
	ContextMap::iterator cit = contexts.find(mContextID);
	if(cit != contexts.end()) {
		ResourceSet &ctx_set = cit->second;
		ResourceSet::iterator it = ctx_set.begin();
		ResourceSet::iterator end = ctx_set.end();
		for(; it != end; ++it) {
			(*it)->destroy();
		}
	}
}



void GPUObject :: contextRegister(int ctx) {
	contextUnregister();
	ContextMap& contexts = getContextMap();
	ResourceMap& resources = getResourceMap();
	contexts[ctx].insert(this);
	resources[this] = ctx;
}

void GPUObject :: contextUnregister() {
	ContextMap& contexts = getContextMap();
	ResourceMap& resources = getResourceMap();
	
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


