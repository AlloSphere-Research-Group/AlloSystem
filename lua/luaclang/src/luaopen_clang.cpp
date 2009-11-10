#include "luaopen_clang.h"
#include "luaglue.h"

#include <string>

#include "llvm/Config/config.h"

#include "clang/AST/ASTContext.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/FileManager.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Driver/Driver.h"
#include "clang/Frontend/CompileOptions.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/ParseAST.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Config/config.h"
#include "llvm/DerivedTypes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Linker.h"
#include "llvm/Module.h"
#include "llvm/ModuleProvider.h"
#include "llvm/Target/Targetdata.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Type.h"
#include "llvm/PassManager.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/System/DynamicLibrary.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/System/DynamicLibrary.h"
#include "llvm/System/Host.h"
#include "llvm/System/Path.h"
#include "llvm/System/Signals.h"

#include <sstream>

using namespace llvm;
struct ForceJITLinking forcejitlinking;

// used by almost all LLVM types:
template<typename T>
int llvm_print(lua_State * L, T * u) {
	std::stringstream s;
	u->print(s);
	lua_pushfstring(L, "%s", s.str().c_str());
	return 1;
}

template<> int llvm_print<Module>(lua_State * L, Module * u) {
	std::stringstream s;
	u->print(s, 0);
	lua_pushfstring(L, "%s", s.str().c_str());
	return 1;
}

/*
	Globals (should they be statics, or per Lua universe?)
*/
static LLVMContext ctx;

/*
	Execution Engine
*/
#pragma mark EE
static ExecutionEngine * EE = 0;

static void registerWithJIT(lua_State * L, Module * module) {
	ExistingModuleProvider * emp = new ExistingModuleProvider(module);

	// register with JIT (create if necessary)
	if (EE == 0) {
		std::string err;
		InitializeNativeTarget();
		EE = ExecutionEngine::createJIT(emp, &err, 0, CodeGenOpt::Default, false);
		if (EE == 0) {
			luaL_error(L, "Failed to create Execution Engine %p: %s\n", EE, err.c_str());
		}
	} else {
		EE->addModuleProvider(emp);
	}
}

static void Lua2GV(lua_State * L, int idx, GenericValue & v, const Type * t) {
	switch(t->getTypeID()) {
		case Type::VoidTyID:
			break;
		case Type::FloatTyID:
			v.FloatVal = luaL_optnumber(L, idx, 0);
			break;
		case Type::DoubleTyID:
			v.DoubleVal = luaL_optnumber(L, idx, 0);
			break;
		case Type::IntegerTyID:
			printf("int\n");
			int i = luaL_optinteger(L, idx, 0);
			v.IntVal = APInt(((IntegerType *)t)->getBitWidth(), i);
			break;
		case Type::PointerTyID:
			v = PTOGV(lua_touserdata(L, idx));
			break;
		case Type::OpaqueTyID:
		case Type::FunctionTyID:
		case Type::StructTyID:
		case Type::ArrayTyID:
		case Type::VectorTyID:
			v.PointerVal = lua_touserdata(L, idx);
			break;
	}
}
static int GV2Lua(lua_State * L, GenericValue & v, const Type * t) {
	switch(t->getTypeID()) {
		case Type::VoidTyID:
			lua_pushnil(L);
			return 1;
		case Type::FloatTyID:
			lua_pushnumber(L, v.FloatVal);
			return 1;
		case Type::DoubleTyID:
			lua_pushnumber(L, v.DoubleVal);
			return 1;
		case Type::IntegerTyID:
			if (v.IntVal.getBitWidth() == 1) {
				lua_pushnumber(L, v.IntVal.getBoolValue());
				return 1;
			}
			lua_pushinteger(L, v.IntVal.getSExtValue());
			return 1;
		case Type::PointerTyID:
		case Type::OpaqueTyID:
		case Type::FunctionTyID:
		case Type::StructTyID:
		case Type::ArrayTyID:
		case Type::VectorTyID:
			lua_pushlightuserdata(L, v.PointerVal);
			return 1;
	}
	return 0;
}

/* 
	Module
*/
#pragma mark Module
template<> const char * Glue<Module>::usr_name() { return "Module"; }
template<> int Glue<Module>::usr_tostring(lua_State * L, Module * u) {
	lua_pushfstring(L, "%s: %s(%p)", usr_name(), u->getModuleIdentifier().c_str(), u);
	return 1;
}
static int module_linkto(lua_State * L) {
	std::string err;
	Module * self = Glue<Module>::checkto(L, 1);
	Module * mod = Glue<Module>::checkto(L, 2);
	llvm::Linker::LinkModules(self, mod, &err);
	if (err.length())
		luaL_error(L, err.c_str());
	return 0;
}
static int ee_call(lua_State * L) {
	Module * m = Glue<Module>::checkto(L, 1);
	if (EE == 0)
		return luaL_error(L, "no execution engine");
	Function * f;
	const char * name;
	if (lua_isstring(L, 2)) {
		name = lua_tostring(L, 2);
		f = m->getFunction(name);
	} else {
		f = Glue<Function>::checkto(L, 2);
		name = f->getNameStr().c_str();
	}
	if (f == 0)
		return luaL_error(L, "function %s not found", name);

	const FunctionType * ft = f->getFunctionType();

	// get args:
	std::vector<GenericValue> gvs(ft->getNumParams());
	for (int i=0; i<ft->getNumParams(); i++) {
		Lua2GV(L, i+3, gvs[i], ft->getParamType(i));
	}
	GenericValue res = EE->runFunction(f, gvs);
	return GV2Lua(L, res, ft->getReturnType());
}
template<> Module * Glue<Module>::usr_new(lua_State * L) {
	return new Module(luaL_checkstring(L, 1), ctx); 
}
template<> void Glue<Module>::usr_push(lua_State * L, Module * u) {
	registerWithJIT(L, u);
}
template<> void Glue<Module> :: usr_mt(lua_State * L) {
	lua_pushcfunction(L, module_linkto); lua_setfield(L, -2, "linkto");
	lua_pushcfunction(L, ee_call); lua_setfield(L, -2, "call");
}


/* 
	ModuleProvider
*/
#pragma mark ModuleProvider
template<> const char * Glue<ModuleProvider>::usr_name() { return "ModuleProvider"; }
template<> int Glue<ModuleProvider>::usr_tostring(lua_State * L, ModuleProvider * u) {
	lua_pushfstring(L, "%s: %s(%p)", usr_name(), u->getModule()->getModuleIdentifier().c_str(), u);
	return 1;
}
template<> ModuleProvider * Glue<ModuleProvider>::usr_new(lua_State * L) {
	Module * m = Glue<Module>::checkto(L, 1);
	return new ExistingModuleProvider(m);
}


#pragma mark Type
template<> const char * Glue<Type>::usr_name() { return "Type"; }
template<> int Glue<Type>::usr_tostring(lua_State * L, Type * t) { return llvm_print(L, t); }
// type call with no argument is a constant constructor
// type call with an argument is a cast operator
//static int type_call(lua_State * L) {
//	const Type * t = Glue<Type>::checkto(L, 1);
//	if (!lua_isuserdata(L, 2)) {
//		// constant initializer:
//		switch (t->getTypeID()) {
//			case Type::VoidTyID:
//				return Glue<Constant>::push(L, ConstantPointerNull::get((const PointerType *)Type::getVoidTy(getGlobalContext())));
//			case Type::FloatTyID:
//			case Type::DoubleTyID:
//				return Glue<Constant>::push(L, ConstantFP::get(t, luaL_optnumber(L, 2, 0.)));
//			case Type::IntegerTyID:
//				return Glue<Constant>::push(L, ConstantInt::get(t, luaL_optinteger(L, 2, 0)));
//		}
//		return luaL_error(L, "Type cannot be constructed as a constant");
//	} else {
//		// cast:
//		Value * v = Glue<Value>::checkto(L, 2);
//		const Type * t2 = v->getType();
//		IRBuilder<> * b = getLModule(L)->getBuilder();
//
//		// can't cast void:
//		if (t->getTypeID() == Type::VoidTyID || t2->getTypeID() == Type::VoidTyID)
//			return luaL_error(L, "Cannot cast to/from Void");
//
//		// ptr to ptr
//		if (t->getTypeID() == Type::PointerTyID && t2->getTypeID() == Type::PointerTyID) {
//			return Glue<Value>::push(L, b->CreateBitCast(v, t, "cast"));
//		}
//
//		// int to float:
//		if (t->isInteger() && t2->isFloatingPoint())
//			return Glue<Value>::push(L, b->CreateFPToSI(v, t, "intcast"));
//		// float to int
//		if (t->isFloatingPoint() && t2->isInteger())
//			return Glue<Value>::push(L, b->CreateSIToFP(v, t, "floatcast"));
//
//		// int to int
//		if (t->isInteger() == t2->isInteger()) {
//			const IntegerType * it = (IntegerType *)t;
//			const IntegerType * it2 = (IntegerType *)t2;
//			if (it->getBitWidth() > it2->getBitWidth()) {
//				return Glue<Value>::push(L, b->CreateZExt(v, it, "trunc"));
//			} else if (it->getBitWidth() < it2->getBitWidth()) {
//				return Glue<Value>::push(L, b->CreateTrunc(v, it, "trunc"));
//			} else {
//				return 1; // no cast required
//			}
//		}
//
//		return luaL_error(L, "unrecognized cast");
//	}
//}

int type_id(lua_State * L) {
	const Type * t = Glue<Type>::checkto(L, 1);
	lua_pushinteger(L, t->getTypeID());
	return 1;
}
int type_isinteger(lua_State * L) {
	const Type * t = Glue<Type>::checkto(L, 1);
	lua_pushboolean(L, t->isInteger());
	return 1;
}
int type_isfloatingpoint(lua_State * L) {
	const Type * t = Glue<Type>::checkto(L, 1);
	lua_pushboolean(L, t->isFloatingPoint());
	return 1;
}
int type_isabstract(lua_State * L) {
	const Type * t = Glue<Type>::checkto(L, 1);
	lua_pushboolean(L, t->isAbstract());
	return 1;
}
int type_issized(lua_State * L) {
	const Type * t = Glue<Type>::checkto(L, 1);
	lua_pushboolean(L, t->isSized());
	return 1;
}
//int type_sizeABI(lua_State * L) {
//	const Type * t = Glue<Type>::checkto(L, 1);
//	ExecutionEngine * ee = LModule::getEE();
//	const TargetData * td = ee->getTargetData();
//	/// getTypeSizeInBits - Return the number of bits necessary to hold the
//	/// specified type.  For example, returns 36 for i36 and 80 for x86_fp80.
//	lua_pushinteger(L, td->getABITypeSize(t));
//	return 1;
//}
int type_sizeinbits(lua_State * L) {
	const Type * t = Glue<Type>::checkto(L, 1);
	const TargetData * td = EE->getTargetData();
	/// getTypeSizeInBits - Return the number of bits necessary to hold the
	/// specified type.  For example, returns 36 for i36 and 80 for x86_fp80.
	lua_pushinteger(L, td->getTypeSizeInBits(t));
	return 1;
}
static int type_pointer(lua_State * L) {
	const Type * t = Glue<Type>::checkto(L, 1);
	int addressSpace = luaL_optinteger(L, 2, 0);
	if (t->getTypeID() == Type::VoidTyID) {
		// special case for void *:
		return Glue<PointerType>::push(L, PointerType::get(Type::getInt8Ty(getGlobalContext()), addressSpace));
	}
	return Glue<PointerType>::push(L, PointerType::get(t, addressSpace));
}
static int type_eq(lua_State * L) {
	//lua::dump(L, "eq");
	const Type * a = Glue<Type>::checkto(L, 1);
	const Type * b = Glue<Type>::checkto(L, 2);
	//printf("%p %p\n", a, b);
	lua_pushboolean(L, a == b);
	return 1;
}
int type_modname(lua_State * L) {
	Module * M = Glue<Module>::checkto(L, 1);
	const Type * t = Glue<Type>::checkto(L, 2);
	lua_pushstring(L, M->getTypeName(t).c_str());
	return 1;
}
template<> void Glue<Type>::usr_mt(lua_State * L) {
	//lua_pushcfunction(L, type_call); lua_setfield(L, -2, "__call");
	lua_pushcfunction(L, type_eq); lua_setfield(L, -2, "__eq");
	lua_pushcfunction(L, type_pointer); lua_setfield(L, -2, "pointer");
	lua_pushcfunction(L, type_pointer); lua_setfield(L, -2, "ptr");
	lua_pushcfunction(L, type_modname); lua_setfield(L, -2, "name");
	lua_pushcfunction(L, type_isinteger); lua_setfield(L, -2, "isinteger");
	lua_pushcfunction(L, type_isfloatingpoint); lua_setfield(L, -2, "isfloatingpoint");
	lua_pushcfunction(L, type_isabstract); lua_setfield(L, -2, "isabstract");
	lua_pushcfunction(L, type_issized); lua_setfield(L, -2, "issized");
	lua_pushcfunction(L, type_sizeinbits); lua_setfield(L, -2, "sizeinbits");
	//lua_pushcfunction(L, type_sizeABI); lua_setfield(L, -2, "size");
	lua_pushcfunction(L, type_id); lua_setfield(L, -2, "id");
}

/*
	StructType : CompositeType : DerivedType : Type
*/
#pragma mark StructType
template<> const char * Glue<StructType>::usr_name() { return "StructType"; }
template<> int Glue<StructType>::usr_tostring(lua_State * L, StructType * t) { return llvm_print<StructType>(L, t); }
template<> StructType * Glue<StructType>::usr_new(lua_State * L) {
	std::vector<const Type *> types;
	if (lua_type(L, 1) == LUA_TTABLE) {
		int ntypes = lua_objlen(L, 1);
		for (int i=1; i<= ntypes; i++) {
			lua_rawgeti(L, 1, i);
			types.push_back(Glue<Type>::checkto(L, -1));
			lua_pop(L, 1);
		}
	}
	bool isPacked = false; // this is true for unions, I think??
	return StructType::get((getGlobalContext()), types, isPacked);
}
static int structtype_len(lua_State * L) {
	StructType * u = Glue<StructType>::checkto(L, 1);
	lua_pushinteger(L, u->getNumElements());
	return 1;
}
static int structtype_getelementtype(lua_State * L) {
	StructType * u = Glue<StructType>::checkto(L, 1);
	int i = luaL_checkinteger(L, 2);
	if (i >= u->getNumElements())
		return luaL_error(L, "StructType has only %d elements (requested %d)", u->getNumElements(), i);
	return Glue<Type>::push(L, (Type *)u->getElementType(i));
}
static int structtype_gettypes(lua_State * L) {
	StructType * u = Glue<StructType>::checkto(L, 1);
	for (int i=0; i< u->getNumElements(); i++) {
		Glue<Type>::push(L, (Type *)u->getElementType(i));
	}
	return u->getNumElements();
}
template<> void Glue<StructType>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, structtype_len); lua_setfield(L, -2, "__len");
	lua_pushcfunction(L, type_eq); lua_setfield(L, -2, "__eq");
	lua_pushcfunction(L, structtype_getelementtype); lua_setfield(L, -2, "type");
	lua_pushcfunction(L, structtype_gettypes); lua_setfield(L, -2, "types");
}

/*
	SequentialType : CompositeType : DerivedType : Type
*/
#pragma mark SequentialType
template<> const char * Glue<SequentialType>::usr_name() { return "SequentialType"; }
template<> int Glue<SequentialType>::usr_tostring(lua_State * L, SequentialType * t) { return llvm_print<SequentialType>(L, t); }
static int sequentialtype_getelementtype(lua_State * L) {
	SequentialType * u = Glue<SequentialType>::checkto(L, 1);
	return Glue<Type>::push(L, (Type *)u->getElementType());
}
template<> void Glue<SequentialType>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, type_eq); lua_setfield(L, -2, "__eq");
	lua_pushcfunction(L, sequentialtype_getelementtype); lua_setfield(L, -2, "type");
}

/*
	PointerType : SequentialType
*/
#pragma mark PointerType
template<> const char * Glue<PointerType>::usr_name() { return "PointerType"; }
template<> int Glue<PointerType>::usr_tostring(lua_State * L, PointerType * t) { return llvm_print<PointerType>(L, t); }
template<> PointerType * Glue<PointerType>::usr_new(lua_State * L) {
	const Type * t = Glue<Type>::checkto(L, 1);
	int addressSpace = luaL_optinteger(L, 2, 0);
	return PointerType::get(t, addressSpace);
}
template<> void Glue<PointerType>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, type_eq); lua_setfield(L, -2, "__eq");
}

/*
	ArrayType : SequentialType
*/
#pragma mark ArrayType
template<> const char * Glue<ArrayType>::usr_name() { return "ArrayType"; }
template<> int Glue<ArrayType>::usr_tostring(lua_State * L, ArrayType * t) { return llvm_print<ArrayType>(L, t); }
template<> ArrayType * Glue<ArrayType>::usr_new(lua_State * L) {
	const Type * t = Glue<Type>::checkto(L, 1);
	int len = luaL_optinteger(L, 2, 1);
	return ArrayType::get(t, len);
}
static int arraytype_len(lua_State * L) {
	ArrayType * u = Glue<ArrayType>::checkto(L, 1);
	lua_pushinteger(L, u->getNumElements());
	return 1;
}
template<> void Glue<ArrayType>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, arraytype_len); lua_setfield(L, -2, "__len");
	lua_pushcfunction(L, type_eq); lua_setfield(L, -2, "__eq");
}

/*
	VectorType : SequentialType
*/
#pragma mark VectorType
template<> const char * Glue<VectorType>::usr_name() { return "VectorType"; }
template<> int Glue<VectorType>::usr_tostring(lua_State * L, VectorType * t) { return llvm_print<VectorType>(L, t); }
template<> VectorType * Glue<VectorType>::usr_new(lua_State * L) {
	const Type * t = Glue<Type>::checkto(L, 1);
	int len = luaL_optinteger(L, 2, 1);
	return VectorType::get(t, len);
}
static int vectortype_len(lua_State * L) {
	VectorType * u = Glue<VectorType>::checkto(L, 1);
	lua_pushinteger(L, u->getNumElements());
	return 1;
}
template<> void Glue<VectorType>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, vectortype_len); lua_setfield(L, -2, "__len");
	lua_pushcfunction(L, type_eq); lua_setfield(L, -2, "__eq");
}

/*
	OpaqueType : DerivedType : Type
*/
#pragma mark OpaqueType
template<> const char * Glue<OpaqueType>::usr_name() { return "OpaqueType"; }
template<> int Glue<OpaqueType>::usr_tostring(lua_State * L, OpaqueType * t) { return llvm_print<OpaqueType>(L, t); }
template<> OpaqueType * Glue<OpaqueType>::usr_new(lua_State * L) {
	return OpaqueType::get(getGlobalContext());
}
template<> void Glue<OpaqueType>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, type_eq); lua_setfield(L, -2, "__eq");
}

/*
	FunctionType : DerivedType : Type
*/
#pragma mark FunctionType
template<> const char * Glue<FunctionType>::usr_name() { return "FunctionType"; }
template<> int Glue<FunctionType>::usr_tostring(lua_State * L, FunctionType * t) { return llvm_print<FunctionType>(L, t); }
template<> FunctionType * Glue<FunctionType>::usr_new(lua_State * L) {
	const Type * ret = Glue<Type>::checkto(L, 1);
	std::vector<const Type *> types;
	int ntypes = lua_gettop(L) - 1;
	for (int i=0; i<ntypes; i++)
		types.push_back(Glue<Type>::checkto(L, i+2));
	bool isVarArg = false; // this is true for unions, I think??
	return FunctionType::get(ret, types, isVarArg);
}
static int functiontype_isvararg(lua_State * L) {
	FunctionType * f = Glue<FunctionType>::checkto(L, 1);
	lua_pushinteger(L, f->isVarArg());
	return 1;
}
static int functiontype_getparamtype(lua_State * L) {
	FunctionType * u = Glue<FunctionType>::checkto(L, 1);
	int i = luaL_checkinteger(L, 2);
	if (i >= u->getNumParams())
		return luaL_error(L, "FunctionType has only %d params (requested %d)", u->getNumParams(), i);
	return Glue<Type>::push(L, (Type *)u->getParamType(i));
}
static int functiontype_getreturntype(lua_State * L) {
	FunctionType * u = Glue<FunctionType>::checkto(L, 1);
	return Glue<Type>::push(L, (Type *)u->getReturnType());
}
static int functiontype_len(lua_State * L) {
	FunctionType * u = Glue<FunctionType>::checkto(L, 1);
	lua_pushinteger(L, u->getNumParams());
	return 1;
}
template<> void Glue<FunctionType>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, type_eq); lua_setfield(L, -2, "__eq");
	lua_pushcfunction(L, functiontype_len); lua_setfield(L, -2, "__len");
	lua_pushcfunction(L, functiontype_isvararg); lua_setfield(L, -2, "isvararg");
	lua_pushcfunction(L, functiontype_getparamtype); lua_setfield(L, -2, "param");
	lua_pushcfunction(L, functiontype_getreturntype); lua_setfield(L, -2, "ret");
	lua_pushcfunction(L, Glue<Function>::create); lua_setfield(L, -2, "__call");
}

/*
	Value
*/
#pragma mark Value
template<> const char * Glue<Value>::usr_name() { return "Value"; }
template<> int Glue<Value>::usr_tostring(lua_State * L, Value * u) { return llvm_print<Value>(L, u); }
template<> Value * Glue<Value>::usr_reinterpret(lua_State * L, int idx) {
	if (lua_isnoneornil(L, idx))
		return 0;
	double n;
	switch (lua_type(L, idx)) {
		case LUA_TBOOLEAN:
			// equiv: const bool;
			return ConstantInt::get((getGlobalContext()), APInt(1, lua_toboolean(L, idx)));
		case LUA_TNUMBER:
			n = lua_tonumber(L, idx);
			if (fmod(n, 1.0) == 0.)
				// equiv: const sint32_t;
				return ConstantInt::get((getGlobalContext()), APInt(32, lua_tonumber(L, idx)));
			else	
				// equiv: const double;
				return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), lua_tonumber(L, idx));
		case LUA_TSTRING:
			// equiv: const char * (null terminated)
			return ConstantArray::get((getGlobalContext()), std::string(lua_tostring(L, idx)), true); // true == null terminate
		case LUA_TUSERDATA:	{	 
			// makes sense only if it is a Value:
			Value * u = Glue<Value>::checkto(L, idx);
			return u;
		}
		case LUA_TLIGHTUSERDATA: // pointers can't be typeless constants, so exchange for null
		case LUA_TNIL:
		default:				// thread, function & table make no sense for an LLVM Value *
			luaL_error(L, "cannot interpret value of type %s\n", lua_typename(L, lua_type(L, idx)));
			break;
	}
	return 0;
}
static int value_type(lua_State * L) {
	Value * u = Glue<Value>::checkto(L);
	return Glue<Type>::push(L, (Type *)u->getType());
}
static int value_name(lua_State * L) {
	Value * u = Glue<Value>::checkto(L);
	if (lua_isstring(L, 2)) {
		const char * name = lua_tostring(L, 2);
		u->setName(name);
		lua_pushvalue(L, 1);
		return 1;
	}
	lua_pushstring(L, u->getNameStr().c_str());
	return 1;
}
static int value_replace(lua_State * L) {
	Value * u = Glue<Value>::checkto(L, 1);
	Value * v = Glue<Value>::checkto(L, 2);
	u->replaceAllUsesWith(v);
	return 0;
}
static int value_uses(lua_State * L) {
	Value * u = Glue<Value>::checkto(L, 1);
	lua_pushinteger(L, u->getNumUses());
	return 1;
}
template<> void Glue<Value>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, value_type); lua_setfield(L, -2, "type");
	lua_pushcfunction(L, value_name); lua_setfield(L, -2, "name");
	lua_pushcfunction(L, value_replace); lua_setfield(L, -2, "replace");
	lua_pushcfunction(L, value_uses); lua_setfield(L, -2, "uses");
}

/*
	Argument : Value
*/
#pragma mark Argument
template<> const char * Glue<Argument>::usr_name() { return "Argument"; }
template<> int Glue<Argument>::usr_tostring(lua_State * L, Argument * u) { return llvm_print<Argument>(L, u); }
static int argument_parent(lua_State * L) {
	Argument * u = Glue<Argument>::checkto(L, 1);
	return Glue<Function>::push(L, u->getParent());
}
static int argument_argno(lua_State * L) {
	Argument * u = Glue<Argument>::checkto(L, 1);
	lua_pushinteger(L, u->getArgNo());
	return 1;
}
static int argument_byval(lua_State * L) {
	Argument * u = Glue<Argument>::checkto(L, 1);
	lua_pushboolean(L, u->hasByValAttr());
	return 1;
}
template<> void Glue<Argument>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, argument_parent); lua_setfield(L, -2, "parent");
	lua_pushcfunction(L, argument_argno); lua_setfield(L, -2, "argno");
	lua_pushcfunction(L, argument_byval); lua_setfield(L, -2, "byval");
}

/*
	Instruction : User : Value
*/
#pragma mark Instruction
template<> const char * Glue<Instruction>::usr_name() { return "Instruction"; }
template<> int Glue<Instruction>::usr_tostring(lua_State * L, Instruction * u) { return llvm_print<Instruction>(L, u); }
static int inst_parent(lua_State * L) {
	Instruction * u = Glue<Instruction>::checkto(L, 1);
	return Glue<BasicBlock>::push(L, u->getParent());
}
static int inst_erase(lua_State * L) {
	Instruction * f = Glue<Instruction>::checkto(L, 1);
	f->eraseFromParent();
	Glue<Instruction>::erase(L, 1);
	return 0;
}
static int inst_opcode(lua_State * L) {
	Instruction * u = Glue<Instruction>::checkto(L, 1);
	lua_pushstring(L, u->getOpcodeName());
	return 1;
}
template<> void Glue<Instruction>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, inst_parent); lua_setfield(L, -2, "parent");
	lua_pushcfunction(L, inst_erase); lua_setfield(L, -2, "erase");
	lua_pushcfunction(L, inst_opcode); lua_setfield(L, -2, "opcode");
}

/*
	PHINode : Instruction
*/
#pragma mark PHINode
template<> const char * Glue<PHINode>::usr_name() { return "PHINode"; }
template<> int Glue<PHINode>::usr_tostring(lua_State * L, PHINode * u) { return llvm_print<PHINode>(L, u); }
static int phi_addincoming(lua_State * L) {
	PHINode * u = Glue<PHINode>::checkto(L, 1);
	Value * v = Glue<Value>::checkto(L, 2);
	BasicBlock * block = Glue<BasicBlock>::checkto(L, 3);
	u->addIncoming(v, block);
	return 0;
}
template<> void Glue<PHINode>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, phi_addincoming); lua_setfield(L, -2, "addincoming");
}


/*
	BasicBlock : Value
*/
#pragma mark BasicBlock
template<> const char * Glue<BasicBlock>::usr_name() { return "BasicBlock"; }
template<> int Glue<BasicBlock>::usr_tostring(lua_State * L, BasicBlock * u) { return llvm_print<BasicBlock>(L, u); }
template<> BasicBlock * Glue<BasicBlock>::usr_new(lua_State * L) {
	const char * name = luaL_optstring(L, 1, "noname");
	Function * parent = Glue<Function>::to(L, 2);
	BasicBlock * insertbefore = Glue<BasicBlock>::to(L, 3);
	return BasicBlock::Create((getGlobalContext()), name, parent, insertbefore);
}
static int block_parent(lua_State * L) {
	BasicBlock * u = Glue<BasicBlock>::checkto(L, 1);
	return Glue<Function>::push(L, u->getParent());
}
static int block_erase(lua_State * L) {
	BasicBlock * f = Glue<BasicBlock>::checkto(L, 1);
	f->eraseFromParent();
	Glue<BasicBlock>::erase(L, 1);
	return 0;
}
static int block_terminator(lua_State * L) {
	BasicBlock * f = Glue<BasicBlock>::checkto(L, 1);
	return Glue<Instruction>::push(L, f->getTerminator());
}
static int block_instruction(lua_State * L) {
	BasicBlock * f = Glue<BasicBlock>::checkto(L, 1);
	int i = luaL_checkinteger(L, 2);
	if (i >= f->size())
		return luaL_error(L, "Function has only %d arguments (requested %d)", f->size(), i);
	BasicBlock::iterator it = f->begin();
	while (i--) it++;
	return Glue<Instruction>::push(L, it);
}
static int block_front(lua_State * L) {
	BasicBlock * f = Glue<BasicBlock>::checkto(L, 1);
	return Glue<Instruction>::push(L, &f->front());
}
static int block_back(lua_State * L) {
	BasicBlock * f = Glue<BasicBlock>::checkto(L, 1);
	return Glue<Instruction>::push(L, &f->back());
}
static int block_len(lua_State * L) {
	BasicBlock * f = Glue<BasicBlock>::checkto(L, 1);
	lua_pushinteger(L, f->size());
	return 1;
}
//static int block_setinsertpoint(lua_State * L) {
//	BasicBlock * block = Glue<BasicBlock>::checkto(L, 1);
//	getLModule(L)->getBuilder()->SetInsertPoint(block);
//	return 0;
//}
//static int block_ret(lua_State * L) {
//	IRBuilder<> * b = getLModule(L)->getBuilder();
//	BasicBlock * block = Glue<BasicBlock>::checkto(L, 1);
//	Value * v = Glue<Value>::to(L, 2);
//	Function * f = block->getParent();
//	if (f) {
//		// check types:
//		const Type * retType = f->getFunctionType()->getReturnType();
//		if (retType->getTypeID() == Type::VoidTyID) {
//			if (v)
//				return luaL_error(L, "current function returns void");
//			return Glue<Instruction>::push(L, b->CreateRetVoid());
//		}
//		if (retType != v->getType())
//			luaL_error(L, "return type mismatch");
//		return Glue<Instruction>::push(L, b->CreateRet(v));
//	}
//	if (v)
//		return Glue<Instruction>::push(L, b->CreateRet(v));
//	return Glue<Instruction>::push(L, b->CreateRetVoid());
//}
template<> void Glue<BasicBlock>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, block_len); lua_setfield(L, -2, "__len");
	lua_pushcfunction(L, block_instruction); lua_setfield(L, -2, "instruction");
	lua_pushcfunction(L, block_parent); lua_setfield(L, -2, "parent");
	lua_pushcfunction(L, block_front); lua_setfield(L, -2, "front");
	lua_pushcfunction(L, block_back); lua_setfield(L, -2, "back");
	lua_pushcfunction(L, block_erase); lua_setfield(L, -2, "erase");
	lua_pushcfunction(L, block_terminator); lua_setfield(L, -2, "terminator");
//	lua_pushcfunction(L, block_setinsertpoint); lua_setfield(L, -2, "setinsertpoint");
//	lua_pushcfunction(L, block_ret); lua_setfield(L, -2, "ret");
}

/*
	Constant : User : Value
*/
#pragma mark Constant
template<> const char * Glue<Constant>::usr_name() { return "Constant"; }
template<> int Glue<Constant>::usr_tostring(lua_State * L, Constant * u) { return llvm_print<Constant>(L, u); }
template<> Constant * Glue<Constant>::usr_reinterpret(lua_State * L, int idx) {
	return (Constant *)Glue<Value>::usr_reinterpret(L, idx);
}
// destroyConstant... todo: call this on all constants registered so far, when the module closes

/*
	GlobalValue : Constant
*/
#pragma mark GlobalValue
template<> const char * Glue<GlobalValue>::usr_name() { return "GlobalValue"; }
template<> int Glue<GlobalValue>::usr_tostring(lua_State * L, GlobalValue * u) { return llvm_print<GlobalValue>(L, u); }
static int global_linkage(lua_State * L) {
	GlobalValue * u = Glue<GlobalValue>::checkto(L, 1);
	if (lua_isnumber(L, 2)) {
		u->setLinkage((GlobalValue::LinkageTypes)lua_tointeger(L, 2));
	}
	lua_pushinteger(L, u->getLinkage());
	return 1;
}
static int global_isdeclaration(lua_State * L) {
	GlobalValue * f = Glue<GlobalValue>::checkto(L, 1);
	lua_pushboolean(L, f->isDeclaration());
	return 1;
}
template<> void Glue<GlobalValue>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, global_linkage); lua_setfield(L, -2, "linkage");
	lua_pushcfunction(L, global_isdeclaration); lua_setfield(L, -2, "isdeclaration");
}
// likely methods:	getParent() (returns a Module *)


/*
	GlobalVariable : GlobalValue
*/
#pragma mark GlobalVariable
template<> const char * Glue<GlobalVariable>::usr_name() { return "GlobalVariable"; }
template<> int Glue<GlobalVariable>::usr_tostring(lua_State * L, GlobalVariable * u) { return llvm_print<GlobalVariable>(L, u); }
template<> GlobalVariable * Glue<GlobalVariable>::usr_new(lua_State * L) {
	Module * m = Glue<Module>::checkto(L, 1);
	const Type * t = Glue<Type>::checkto(L, 2);
	const char * name  = luaL_checkstring(L, 3);
	bool isConstant = lua_toboolean(L, 4);
	Constant * initializer = 0;
	if (lua_isuserdata(L, 5)) {
		initializer = Glue<Constant>::to(L, 5);
		if (initializer->getType() != t) {
			luaL_error(L, "initializer must match type");
			return 0;
		}
	}
	GlobalValue::LinkageTypes lt = (GlobalValue::LinkageTypes)luaL_optinteger(L, 5, GlobalValue::ExternalLinkage);
	return new GlobalVariable((getGlobalContext()), t, isConstant, lt, initializer, name, m);
}
static int globalvar_erase(lua_State * L) {
	GlobalVariable * u = Glue<GlobalVariable>::checkto(L, 1);
	u->eraseFromParent();
	Glue<GlobalVariable>::erase(L, 1);
	return 0;
}
static int globalvar_isconstant(lua_State * L) {
	GlobalVariable * u = Glue<GlobalVariable>::checkto(L, 1);
	lua_pushboolean(L, u->isConstant());
	return 1;
}
static int globalvar_initializer(lua_State * L) {
	GlobalVariable * u = Glue<GlobalVariable>::checkto(L, 1);
	if (u->hasInitializer())
		return Glue<Constant>::push(L, u->getInitializer());
	return 0;
}
template<> void Glue<GlobalVariable>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, globalvar_erase); lua_setfield(L, -2, "erase");
	lua_pushcfunction(L, globalvar_initializer); lua_setfield(L, -2, "initializer");
	lua_pushcfunction(L, globalvar_isconstant); lua_setfield(L, -2, "isconstant");
}

/*
	Function : GlobalValue
*/
#pragma mark Function
template<> const char * Glue<Function>::usr_name() { return "Function"; }
template<> int Glue<Function>::usr_tostring(lua_State * L, Function * u) { return llvm_print<Function>(L, u); }
template<> Function * Glue<Function>::usr_new(lua_State * L) {
	Module * M = Glue<Module>::checkto(L, 1);
	// if argument is a string, then search for a pre-existing function of that name
	if (lua_isstring(L, 2))
		return M->getFunction(lua_tostring(L, 2));
	// else generate a new function:
	const FunctionType * ft = Glue<FunctionType>::checkto(L, 2);
	std::string name  = luaL_checkstring(L, 3);
	GlobalValue::LinkageTypes lt = (GlobalValue::LinkageTypes)luaL_optinteger(L, 4, GlobalValue::ExternalLinkage);
	bool noAutoName = (bool)lua_isboolean(L, 4) && (bool)lua_toboolean(L, 5); // default false
	Function * F = Function::Create(ft, lt, name, M);
	if (noAutoName && F->getName() != name) {
		F->eraseFromParent();
		luaL_error(L, "Function %s already exists", name.c_str());
		return 0;
	}
	int i=0;
	Function::arg_iterator AI = F->arg_begin();
	for (; i < F->getFunctionType()->getNumParams(); ++AI, ++i) {
		char argname[16];
		sprintf(argname, "arg%i", i);
		AI->setName(argname);
	}
	return F;
}
static int function_intrinsic(lua_State * L) {
	Function * f = Glue<Function>::checkto(L, 1);
	lua_pushinteger(L, f->getIntrinsicID());
	return 1;
}
static int function_deletebody(lua_State * L) {
	Function * f = Glue<Function>::checkto(L, 1);
	f->deleteBody();
	return 0;
}
static int function_erase(lua_State * L) {
	Function * f = Glue<Function>::checkto(L, 1);
	f->eraseFromParent();
	Glue<Function>::erase(L, 1);
	return 0;
}
static int function_callingconv(lua_State * L) {
	Function * f = Glue<Function>::checkto(L, 1);
	if (lua_isnumber(L, 2)) {
		unsigned cc = lua_tonumber(L, 2);
		f->setCallingConv(cc);
	}
	lua_pushinteger(L, f->getCallingConv());
	return 1;
}
static int function_argument(lua_State * L) {
	Function * f = Glue<Function>::checkto(L, 1);
	int i = luaL_checkinteger(L, 2);
	if (i >= f->getFunctionType()->getNumParams())
		return luaL_error(L, "Function has only %d arguments (requested %d)", f->getFunctionType()->getNumParams(), i);
	Function::arg_iterator it = f->arg_begin();
	while (i--) it++;
	return Glue<Argument>::push(L, it);
}
static int function_len(lua_State * L) {
	Function * f = Glue<Function>::checkto(L, 1);
	lua_pushinteger(L, f->size());
	return 1;
}
static int function_block(lua_State * L) {
	Function * f = Glue<Function>::checkto(L, 1);
	int i = luaL_checkinteger(L, 2);
	if (i >= f->size())
		return luaL_error(L, "Function has only %d blocks (requested %d)", f->size(), i);
	Function::iterator it = f->begin();
	while (i--) it++;
	return Glue<BasicBlock>::push(L, it);
}
static int function_verify(lua_State * L) {
	Function * u = Glue<Function>::to(L, 1);
	lua_pushboolean(L, u && (verifyFunction(*u, ReturnStatusAction) == false));
	return 1;
}
// trick here is knowing what the right
static int function_getptr(lua_State * L) {
	Function * u = Glue<Function>::to(L, 1);
	void * f = EE->getPointerToFunction(u);
	lua_pushlightuserdata(L, f);
	return 1;
}
//static int function_optimize(lua_State * L) {
//	Function * f = Glue<Function>::checkto(L, 1);
//	lua_pushboolean(L, getLModule(L)->optimize(f));
//	return 1;
//}
static int function_pushcfunction(lua_State * L) {
	Function * u = Glue<Function>::checkto(L, 1);
	void * f = EE->getPointerToFunction(u);
	if(f) { // && u->getFunctionType() != luaFunctionTy) {
		lua_pushcfunction(L, (lua_CFunction)f);
		return 1;
	}
	else {
		return 0;
	}
}
template<> void Glue<Function>::usr_mt(lua_State * L) {
	lua_pushcfunction(L, function_len); lua_setfield(L, -2, "__len");
	lua_pushcfunction(L, function_intrinsic); lua_setfield(L, -2, "intrinsic");
	lua_pushcfunction(L, function_deletebody); lua_setfield(L, -2, "deletebody");
	lua_pushcfunction(L, function_erase); lua_setfield(L, -2, "erase");
	lua_pushcfunction(L, function_callingconv); lua_setfield(L, -2, "callingconv");
	lua_pushcfunction(L, function_argument); lua_setfield(L, -2, "argument");
	lua_pushcfunction(L, function_block); lua_setfield(L, -2, "block");
	lua_pushcfunction(L, function_verify); lua_setfield(L, -2, "verify");
	lua_pushcfunction(L, function_getptr); lua_setfield(L, -2, "getptr");
	//lua_pushcfunction(L, function_optimize); lua_setfield(L, -2, "optimize");
	lua_pushcfunction(L, function_pushcfunction); lua_setfield(L, -2, "pushcfunction");
}


#pragma mark Clang
using namespace clang;
using namespace clang::driver;

class LuaDiagnosticPrinter : public DiagnosticClient {
	lua_State * L;
public:
	LuaDiagnosticPrinter(lua_State * L)
    : L(L) {}
	virtual ~LuaDiagnosticPrinter() {}
	
	virtual void HandleDiagnostic(Diagnostic::Level Level,
                                const DiagnosticInfo &Info) {
		std::ostringstream OS;

		OS << "clang: ";

		switch (Level) {
			case Diagnostic::Ignored: assert(0 && "Invalid diagnostic type");
			case Diagnostic::Note:    OS << "note: "; break;
			case Diagnostic::Warning: OS << "warning: "; break;
			case Diagnostic::Error:   OS << "error: "; break;
			case Diagnostic::Fatal:   OS << "fatal error: "; break;
		}

		llvm::SmallString<100> OutStr;
		Info.FormatDiagnostic(OutStr);
		OS.write(OutStr.begin(), OutStr.size());
		
		const CodeModificationHint * hint = Info.getCodeModificationHints();
		if (hint) {
			OS << hint->CodeToInsert;
		}
		
		lua_pushfstring(L, OS.str().c_str());
	}
};


int compile(lua_State * L) {

	std::string csource = luaL_checkstring(L, 1);
	std::string srcname = luaL_optstring(L, 2, "untitled");
	// todo: set include search paths
	lua_settop(L, 0);
	
	MemoryBuffer * buffer = MemoryBuffer::getMemBufferCopy(csource.c_str(), csource.c_str() + csource.size(), srcname.c_str());
	if (!buffer) {
		luaL_error(L, "couldn't load %s\n", srcname.c_str());
		return 0;
	}
	
	LangOptions lang;
	TargetInfo * target = TargetInfo::CreateTargetInfo(llvm::sys::getHostTriple());

	LuaDiagnosticPrinter client(L);
	Diagnostic diags(&client);
	SourceManager sm;
	FileManager fm;
	HeaderSearch headers(fm);
	Preprocessor pp(diags, lang, *target, sm, headers);
	
	sm.createMainFileIDForMemBuffer(buffer);
	
	IdentifierTable idents(lang);
	SelectorTable selects;
	Builtin::Context builtin(*target);
	ASTContext context(lang, sm, *target, idents, selects, builtin);
	CompileOptions copts; // e.g. optimizations
	CodeGenerator * codegen = CreateLLVMCodeGen(diags, srcname, copts, ctx);
	
	
	ParseAST(pp, codegen, context, false); // last flag is verbose statist
	Module * cmodule = codegen->ReleaseModule(); // or GetModule() if we want to reuse it?
	if (cmodule) {
		// link with other module? JIT?
		//lua_pushboolean(L, true);
		Glue<Module>::push(L, cmodule);
	} else {
		lua_pushboolean(L, false);
		lua_insert(L, 1);
		// diagnose?
		unsigned count = diags.getNumDiagnostics();
		return count+1;
	}

	delete codegen;
	delete target;
	return 1;
}

#pragma mark luaopen_clang
int luaopen_clang(lua_State * L) {

	// too damn useful to not have around:
	llvm::sys::DynamicLibrary::AddSymbol("printf", (void *)printf);
	llvm::InitializeNativeTarget();

	const char * libname = lua_tostring(L, -1);
	struct luaL_reg lib[] = {
		{"compile", compile},
		{NULL, NULL},
	};
	luaL_register(L, libname, lib);
	
	
	Glue<llvm::Module>::publish(L); 
	Glue<llvm::ModuleProvider>::publish(L);
	
	Glue<llvm::Type>::publish(L, true);
	Glue<llvm::StructType>::publish(L, true, Glue<llvm::Type>::usr_name()); 
	Glue<llvm::SequentialType>::publish(L, true, Glue<llvm::Type>::usr_name()); 
	Glue<llvm::PointerType>::publish(L, true, Glue<llvm::SequentialType>::usr_name()); 
	Glue<llvm::ArrayType>::publish(L, true, Glue<llvm::SequentialType>::usr_name()); 
	Glue<llvm::VectorType>::publish(L, true, Glue<llvm::SequentialType>::usr_name());
	Glue<llvm::OpaqueType>::publish(L, true, Glue<llvm::Type>::usr_name());
	Glue<llvm::FunctionType>::publish(L, true, Glue<llvm::Type>::usr_name());
		
	Glue<llvm::Value>::publish(L, false);
	Glue<llvm::Argument>::publish(L, true, Glue<llvm::Value>::usr_name());
	Glue<llvm::Instruction>::publish(L, true, Glue<llvm::Value>::usr_name()); 
	Glue<llvm::PHINode>::publish(L, true, Glue<llvm::Instruction>::usr_name()); 
	Glue<llvm::BasicBlock>::publish(L, true, Glue<llvm::Value>::usr_name());
	Glue<llvm::Constant>::publish(L, true, Glue<llvm::Value>::usr_name()); 
	Glue<llvm::GlobalValue>::publish(L, true, Glue<llvm::Constant>::usr_name()); 
	Glue<llvm::Function>::publish(L, true, Glue<llvm::GlobalValue>::usr_name());
	Glue<llvm::GlobalVariable>::publish(L, true, Glue<llvm::GlobalValue>::usr_name());
	
	lua_pushstring(L, "main");
	lua_insert(L, 1);
	Glue<llvm::Module>::create(L);
	lua_setfield(L, -2, "main");
	
	return 1;
}