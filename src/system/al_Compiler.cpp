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
#include "clang/Frontend/FrontendOptions.h"
//#include "clang/Frontend/HeaderSearchOptions.h"
//#include "clang/Frontend/PreprocessorOptions.h"
#include "clang/Frontend/Utils.h"
//#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
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
	virtual void NotifyFreeingMachineCode(void *OldPtr) {
		printf("JIT freed %p\n", OldPtr);
	}
};
static JITListener gJITEventListener;

static void llvmErrorHandler(void * user_data, const std::string& reason) {
	printf("llvm error %s\n", reason.data());
}

static void llvmInit() {
	static bool initialized = false;
	if (!initialized) {
		llvm::InitializeAllTargets();
		llvm::InitializeAllAsmPrinters();
		//llvm::InitializeAllAsmParsers();
		llvm::install_fatal_error_handler(llvmErrorHandler, NULL);
		initialized = true;
	}
}


namespace al {

class ModuleImpl {
public:
	llvm::Module * module;
	llvm::LLVMContext llvmContext;
	
	ModuleImpl() : module(NULL) {}
	
	bool link(llvm::Module * child) {
		std::string err;
		if (module == NULL) {
			module = child;
		} else {
			llvm::Linker::LinkModules(module, child, &err);
		}
		if (err.length()) {
			printf("link error %s\n", err.data());
			return false;
		}
		return true;
	}
};

Compiler::Compiler() : mImpl(NULL) { llvmInit(); }

Compiler::~Compiler() {
	clear();
}

void Compiler :: clear() {
	if (mImpl) {
		if (mImpl->module)
			delete mImpl->module;
		delete mImpl;
		mImpl = NULL;
	}
}

bool Compiler :: compile(std::string code) {
	if (!mImpl) mImpl = new ModuleImpl;
	
	const char * src = code.data();
	llvm::StringRef input_data(src);
	llvm::StringRef buffer_name("src");
	llvm::MemoryBuffer *buffer = llvm::MemoryBuffer::getMemBufferCopy(input_data, buffer_name);
	if(!buffer) {
		printf("couldn't create buffer\n");
	}
	
	CompilerInstance CI;
	CI.createDiagnostics(0, NULL);
	Diagnostic & Diags = CI.getDiagnostics();	
	TextDiagnosticBuffer * client = new TextDiagnosticBuffer();
	Diags.setClient(client);
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
	for (unsigned int i=0; i<options.system_includes.size(); i++) {
		headeropts.AddPath(options.system_includes[i], clang::frontend::Angled, true, false, false/* true ? */);
	}
	for (unsigned int i=0; i<options.user_includes.size(); i++) {
		headeropts.AddPath(options.user_includes[i], clang::frontend::Quoted, true, false, false/* true ? */);
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
	CodeGenerator * codegen = CreateLLVMCodeGen(Diags, "mymodule", CGO, mImpl->llvmContext );

	
	ParseAST(CI.getPreprocessor(),
			codegen,
			CI.getASTContext(),
			/* PrintState= */ false,
			true,
			0);
	
	llvm::Module* module = codegen->ReleaseModule();
	delete codegen;
	
	if (module) {
		mImpl->link(module);
		return true;
	}
	
	printf("compile errors\n");
	
	int ecount = 0;
	
	for(TextDiagnosticBuffer::const_iterator it = client->err_begin();
		it != client->err_end();
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
	clear();
	return false;
}

bool Compiler :: readbitcode(std::string path) {
	//if (!mImpl) mImpl = new ModuleImpl; //etc.
	printf("readbitcode: not yet implemented\n");
	return true;
}

void Compiler :: dump() {
	if (mImpl) mImpl->module->dump();
}

JIT * Compiler :: jit() {
	std::string err;
	if (mImpl) {
		if (EE==0) {
			EE = llvm::ExecutionEngine::createJIT(
				mImpl->module,	
				&err,		// error string
				0,			// JITMemoryManager
				llvm::CodeGenOpt::Default,	// JIT slowly (None, Default, Aggressive)
				false		// allocate GlobalVariables separately from code
			);
			if (EE == 0) {
				printf("Failed to create Execution Engine: %s", err.data());
				return 0;
			}
			
			// turn this off when not debugging:
			EE->RegisterJITEventListener(&gJITEventListener);
			//EE->InstallLazyFunctionCreator(lazyfunctioncreator);
			//EE->DisableGVCompilation();
		
		} else {
			EE->addModule(mImpl->module);
		}
		EE->runStaticConstructorsDestructors(mImpl->module, false);
		// create JIT and transfer ownership of module to it:
		JIT * jit = new JIT;
		jit->mImpl = mImpl;
		mImpl = NULL;
		return jit;
	}
	return NULL;
}

JIT::JIT() {}

JIT::~JIT() {
	/* free any statics allocated in the code */
	EE->runStaticConstructorsDestructors(mImpl->module, true);
	
	/*	Removing the functions one by one. */
	llvm::Module::FunctionListType & flist = mImpl->module->getFunctionList();
	for (llvm::Module::FunctionListType::iterator iter= flist.begin(); iter != flist.end(); iter++) {
		//printf("function %s %d\n", iter->getName().data(), iter->isIntrinsic());
		EE->freeMachineCodeForFunction(iter);
	}
	EE->clearGlobalMappingsFromModule(mImpl->module);
	
	/* EE forgets about module */
	EE->removeModule(mImpl->module);	
	
	/* should be safe */
	delete mImpl->module;
	delete mImpl;
}

void * JIT :: getfunctionptr(std::string funcname) {
	llvm::StringRef fname = llvm::StringRef(funcname);
	llvm::Function * f = mImpl->module->getFunction(fname);
	if (f) {
		return EE->getPointerToFunction(f);
	}
	return NULL;
}
void * JIT :: getglobalptr(std::string globalname) {
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

void Compiler :: optimize(std::string opt) {
	printf("Compiler optimizations not yet enabled\n");
}	

} // al::
