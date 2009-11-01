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

#include "llvm/Module.h"
#include "llvm/ModuleProvider.h"
#include "llvm/Type.h"
#include "llvm/DerivedTypes.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Config/config.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/System/DynamicLibrary.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/System/Host.h"
#include "llvm/System/Path.h"
#include "llvm/System/Signals.h"

// code-gen:
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/IRBuilder.h"

// optimizer:
#include "llvm/PassManager.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Scalar.h"

// jit:
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Target/TargetData.h"
#include "llvm/System/DynamicLibrary.h"

#include <sstream>

using namespace llvm;

/*
	Globals (should they be statics, or per Lua universe?)
*/
static LLVMContext ctx;
static ExecutionEngine * EE = 0;

/* 
	Module
*/
#pragma mark Module
template<> const char * Glue<Module>::usr_name() { return "Module"; }
template<> int Glue<Module>::usr_tostring(lua_State * L, Module * u) {
	lua_pushfstring(L, "%s: %s(%p)", usr_name(), u->getModuleIdentifier().c_str(), u);
	return 1;
}
template<> Module * Glue<Module>::usr_new(lua_State * L) {
	return new Module(luaL_checkstring(L, 1), ctx); 
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


int luaopen_clang(lua_State * L) {

	// too damn useful to not have around:
	llvm::sys::DynamicLibrary::AddSymbol("printf", (void *)printf);


	const char * libname = lua_tostring(L, -1);
	struct luaL_reg lib[] = {
		{"compile", compile},
		{NULL, NULL},
	};
	luaL_register(L, libname, lib);
	
	
	Glue<Module>::publish(L); 
	Glue<ModuleProvider>::publish(L);
	
	lua_pushstring(L, "main");
	lua_insert(L, 1);
	Glue<Module>::create(L);
	lua_setfield(L, -2, "main");
	
	return 1;
}