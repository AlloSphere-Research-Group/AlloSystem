#include "system/al_Compiler.hpp"

//#include "clang/Checker/BugReporter/PathDiagnostic.h"
#include "clang/AST/ASTContext.h"
//#include "clang/AST/Decl.h"
#include "clang/Basic/Builtins.h"
//#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/TargetInfo.h"
//#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/SourceManager.h"
//#include "clang/Basic/FileManager.h"
//#include "clang/CodeGen/CodeGenOptions.h"
#include "clang/CodeGen/ModuleBuilder.h"
//#include "clang/Driver/Driver.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
//#include "clang/Frontend/FrontendOptions.h"
//#include "clang/Frontend/HeaderSearchOptions.h"
//#include "clang/Frontend/PreprocessorOptions.h"
#include "clang/Frontend/Utils.h"
//#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/ParseAST.h"
//#include "clang/Sema/CodeCompleteConsumer.h"

//#include "llvm/ADT/OwningPtr.h"
//#include "llvm/ADT/SmallPtrSet.h"
//#include "llvm/ADT/SmallString.h"
//#include "llvm/ADT/StringExtras.h"
//#include "llvm/Analysis/Verifier.h"
//#include "llvm/Bitcode/ReaderWriter.h"
//#include "llvm/Analysis/Verifier.h"
//#include "llvm/Config/config.h"
//#include "llvm/DerivedTypes.h"
//#include "llvm/ExecutionEngine/ExecutionEngine.h"
//#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/Linker.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
//#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetSelect.h"
//#include "llvm/Transforms/Scalar.h"
#include "llvm/Type.h"
//#include "llvm/PassManager.h"
//#include "llvm/Support/Allocator.h"
#include "llvm/Support/ErrorHandling.h"
//#include "llvm/Support/IRBuilder.h"
//#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
//#include "llvm/Support/PassNameParser.h"
//#include "llvm/Support/PrettyStackTrace.h"
//#include "llvm/Support/raw_ostream.h"
//#include "llvm/System/DynamicLibrary.h"
//#include "llvm/System/Host.h"
//#include "llvm/System/Path.h"
//#include "llvm/System/Signals.h"
//#include "llvm/Transforms/IPO.h"
//#include "llvm/Transforms/Scalar.h"
//#include "llvm/ValueSymbolTable.h"

using namespace clang;

struct ForceJITLinking forcejitlinking;
static llvm::ExecutionEngine * EE = 0;

class JITListener : public llvm::JITEventListener {
public:
	JITListener() : llvm::JITEventListener() {}
	virtual ~JITListener() {}
	virtual void NotifyFunctionEmitted(const llvm::Function &F,
										 void *Code, size_t Size,
										 const llvm::JITEvent_EmittedFunctionDetails &Details) {
		printf("JIT emitted Function %s at %p, size %d\n", 
			F.getName().data(), Code, (int)Size);
	}
	virtual void NotifyFreeingMachineCode(const llvm::Function &F, void *OldPtr) {
		printf("JIT freed Function %s at %p\n", 
			F.getName().data(), OldPtr);
	}
};
static JITListener gJITEventListener;

static void llvmErrorHandler(void * user_data, const std::string& reason) {
	printf("llvm error %s\n", reason.data());
}


namespace al {

class CompilerImpl {
public:
	llvm::Module * mainModule;
	llvm::LLVMContext llvmContext;
	
	CompilerImpl(std::string name) {
		mainModule = new llvm::Module(name, llvmContext);
		jit(mainModule);
	}
	
	~CompilerImpl() {
		/*	Removing the functions one by one. */
		llvm::Module::FunctionListType & flist = mainModule->getFunctionList();
		for (llvm::Module::FunctionListType::iterator iter= flist.begin(); iter != flist.end(); iter++) {
			//printf("function %s %d\n", iter->getName().data(), iter->isIntrinsic());
			EE->freeMachineCodeForFunction(iter);
		}
		/* free any statics allocated in the code */
		EE->runStaticConstructorsDestructors(mainModule, true);
		EE->clearGlobalMappingsFromModule(mainModule);
		/* EE forgets about module */
		EE->removeModule(mainModule);	
		/* should be safe */
		delete mainModule;
	}
	
	llvm::Module * module() { return mainModule; }
	
	void jit(llvm::Module * module) {
		std::string err;
		if (EE==0) {
			llvm::InitializeAllTargets();
			llvm::InitializeAllAsmPrinters();
			llvm::llvm_install_error_handler(llvmErrorHandler, NULL);
		
			EE = llvm::ExecutionEngine::createJIT(
				mainModule,	// module provider
				&err,		// error string
				0,			// JITMemoryManager
				llvm::CodeGenOpt::Default,	// JIT slowly (None, Default, Aggressive)
				false		// allocate GlobalVariables separately from code
			);
			if (EE == 0) {
				printf("Failed to create Execution Engine: %s", err.data());
			}
			
			// turn this off when not debugging:
			EE->RegisterJITEventListener(&gJITEventListener);
			//EE->InstallLazyFunctionCreator(lazyfunctioncreator);
			//EE->DisableGVCompilation();
		} else {
			EE->addModule(module);
		}
		EE->runStaticConstructorsDestructors(module, false);
		//EE->DisableLazyCompilation();
	}
	
	void link(llvm::Module * child) {
		std::string err;
		llvm::Linker::LinkModules(mainModule, child, &err);
		if (err.length()) {
			printf("link error %s\n", err.data());
		}
	}
	
	bool compile(std::string code, Compiler::Options& options) {
		const char * src = code.data();
		llvm::MemoryBuffer *buffer = llvm::MemoryBuffer::getMemBufferCopy(src, src+strlen(src), "src");
		if(!buffer) {
			printf("couldn't create buffer\n");
		}
		
		CompilerInstance CI;
		CI.createDiagnostics(0, NULL);
		Diagnostic & Diags = CI.getDiagnostics();	
		TextDiagnosticBuffer client;
		Diags.setClient(&client);
		CompilerInvocation::CreateFromArgs(CI.getInvocation(), NULL, NULL, Diags);
		
	//	// list standard invocation args:
	//	std::vector<std::string> Args;
	//	Compiler.getInvocation().toArgs(Args);
	//	for (int i=0; i<Args.size(); i++) {
	//		printf("%d %s\n", i, Args[i].data());
	//	}
		

		
		LangOptions& lang = CI.getInvocation().getLangOpts();
		// The fateful line
		lang.CPlusPlus = options.CPlusPlus;
		lang.Bool = 1;
		lang.BCPLComment = 1;
		lang.RTTI = 0;
		lang.PICLevel = 1;
		

		CI.createSourceManager();
		CI.getSourceManager().createMainFileIDForMemBuffer(buffer);
		CI.createFileManager();
		
		// Create the target instance.
		CI.setTarget(TargetInfo::CreateTargetInfo(CI.getDiagnostics(), CI.getTargetOpts()));
	//	TargetOptions targetopts = Compiler.getTargetOpts();
	//	printf("triple %s\n", targetopts.Triple.data());
	//	printf("CPU %s\n", targetopts.CPU.data());
	//	printf("ABI %s\n", targetopts.ABI.data());
	//	for (int i=0; i<targetopts.Features.size(); i++)
	//		printf("Feature %s\n", targetopts.Features[i].data());
		
		CI.createPreprocessor();
		Preprocessor &PP = CI.getPreprocessor();
		
		// Header paths:
		HeaderSearchOptions& headeropts = CI.getHeaderSearchOpts();
		for (int i=0; i<options.system_includes.size(); i++) {
			headeropts.AddPath(options.system_includes[i], clang::frontend::Angled, true, false);
		}
		for (int i=0; i<options.user_includes.size(); i++) {
			headeropts.AddPath(options.user_includes[i], clang::frontend::Quoted, true, false);
		}
		ApplyHeaderSearchOptions(PP.getHeaderSearchInfo(), headeropts, lang, CI.getTarget().getTriple());
		
		PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(), PP.getLangOptions().NoBuiltin);
		
				
		
		CI.createASTContext();
	//	llvm::SmallVector<const char *, 32> BuiltinNames;
	//	printf("NoBuiltins: %d\n", Compiler.getASTContext().getLangOptions().NoBuiltin);
	//	Compiler.getASTContext().BuiltinInfo.GetBuiltinNames(BuiltinNames, Compiler.getASTContext().getLangOptions().NoBuiltin);
	//	for (int i = 0; i<BuiltinNames.size(); i++) {
	//		printf("Builtin %s\n", BuiltinNames[i]);
	//	}
		
				
		CodeGenOptions CGO;
		CodeGenerator * codegen = CreateLLVMCodeGen(Diags, "mymodule", CGO,llvmContext );

		
		ParseAST(CI.getPreprocessor(),
				codegen,
				CI.getASTContext(),
				/* PrintState= */ false,
				true,
				0);
		
		llvm::Module* module = codegen->ReleaseModule();
		delete codegen;
		
		if (module) {
			jit(module);
			link(module);
			return true;
		}
		
		printf("compile errors\n");
		
		int ecount = 0;
		for(TextDiagnosticBuffer::const_iterator it = client.err_begin();
			it != client.err_end();
			++it)
		{
			FullSourceLoc SourceLoc = FullSourceLoc(it->first, CI.getSourceManager());
			int LineNum = SourceLoc.getInstantiationLineNumber();
			int ColNum = SourceLoc.getInstantiationColumnNumber();
			const char * bufname = SourceLoc.getManager().getBufferName(SourceLoc);
			printf("error %s line %d col %d: %s\n", bufname, LineNum, ColNum, it->second.data());
			ecount++;
			if(ecount > 250) break;
		}
		return false;
	}
};
	
} // al::

using namespace al;

Compiler::Compiler(std::string name) {
	mImpl = new CompilerImpl(name);
}

Compiler::~Compiler() {
	delete mImpl;
}

bool Compiler :: compile(std::string code) {
	return mImpl->compile(code, options);
}

bool Compiler :: readbitcode(std::string path) {
	return true;
}

void * Compiler :: getfunctionptr(std::string funcname) {
	llvm::Function * f = mImpl->module()->getFunction(funcname);
	if (f) {
		return EE->getPointerToFunction(f);
	}
	return NULL;
}
void * Compiler :: getglobalptr(std::string globalname) {
	return NULL;
}

//std::vector<std::string> Compiler :: getfunctionnames() {
//	mImpl->module();
//}
//std::vector<std::string> Compiler :: getglobalnames() {
//	
//}

bool Compiler :: writebitcode(std::string path) {
	return true;
}	

