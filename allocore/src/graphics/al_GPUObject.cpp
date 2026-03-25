#include "allocore/graphics/al_GPUObject.hpp"
#include <map>
#include <set>

using namespace al;

typedef std::set<al::GPUObject *>		ResourceSet;	// set of resources
typedef std::map<int, ResourceSet>		ContextMap;		// context ID to resource set
typedef std::map<al::GPUObject *, int>	ResourceMap;	// resource to context ID
typedef std::map<int, al::GPUContext *>	Contexts;		// context ID to context object

template<class T>
T& singleton(){ static T t; return t; }

ContextMap& getContextMap(){ return singleton<ContextMap>(); }
ResourceMap& getResourceMap(){ return singleton<ResourceMap>(); }
Contexts& getContexts(){ return singleton<Contexts>(); }

int getNextContextID(){
	static int nextID = GPUContext::defaultContextID();
	int result = nextID++;
	//printf("GPUContext: created new context id %d\n", result);
	return result;
}


GPUContext::GPUContext()
:	mContextID(getNextContextID())
{
	getContexts()[mContextID] = this;
}

GPUContext::~GPUContext(){
	// call destroy on all registered GPUObjects
	contextDestroy();

	// remove self from global list of contexts
	auto& C = getContexts();
	auto it = C.find(mContextID);
	if(it != C.end()) C.erase(it);
}

/*static*/ int GPUContext::defaultContextID(){
	// Note: we reserve 0 for an invalid context
	return 1;
}

/*
Scenario:
a. GPUObjects constructed with default context (= 1)
b. Window 1 constructed; assigned context id 1
c. Window 2 constructed; assigned context id 2

GPUObjects need to be handled by Window 2

Problem:
Do we change context id of GPUObjects to 2 or change Window 2 context id to 1?

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
		auto it = C.find(dfID);

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

template <class Func>
void forEachResourceInContext(int contextID, Func f){
	ContextMap& contexts = getContextMap();
	auto it = contexts.find(contextID);
	if(it != contexts.end()) {
		ResourceSet& resources = it->second;
		for(auto * r : resources) f(*r);
	}
}

void GPUContext::contextCreate(){ //printf("GPUContext::contextCreate %d\n", mContextID);
	forEachResourceInContext(mContextID, [](auto& r){
		r.create();
		//printf("object %s %lu %p\n", typeid(r).name(), r.id(), &r); fflush(stdout);
	});
}

void GPUContext::contextDestroy(){ //printf("GPUContext::contextDestroy %d\n", mContextID);
	forEachResourceInContext(mContextID, [](auto& r){
		r.destroy();
	});
}




GPUObject::GPUObject(int ctx)
{	contextRegister(ctx); }

GPUObject::GPUObject(GPUContext& ctx)
:	GPUObject(ctx.contextID())
{}

GPUObject::~GPUObject(){
	contextUnregister();
}

void GPUObject::validate(){
	if(mResubmit){
		destroy();
		mResubmit=false;
	}
	if(!created()) create();
}

void GPUObject::invalidate(){
	mResubmit = true;
}

bool GPUObject::created() const {
	return id() != 0;
}

void GPUObject::create(){
	if(created()){ destroy(); }
	onCreate();
}

void GPUObject::destroy(){
	if(created()) onDestroy();
	mID=0;
}

void GPUObject::contextRegister(int ctx) {
	contextUnregister();
	getContextMap()[ctx].insert(this);
	getResourceMap()[this] = ctx;
}

void GPUObject::contextUnregister() {
	auto& contexts = getContextMap();
	auto& resources = getResourceMap();

	ResourceMap::iterator rit = resources.find(this);
	if(rit != resources.end()) {
		ContextMap::iterator it = contexts.find( rit->second );
		if(it != contexts.end()) {
			ResourceSet& ctx_set = it->second;
			ResourceSet::iterator sit = ctx_set.find(this);
			if(sit != ctx_set.end()) {
				ctx_set.erase(sit);
			}
		}
		resources.erase(rit);
	}
}


