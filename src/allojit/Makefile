#=========================================================================
# AlloJIT main makefile
#=========================================================================


include ../../Makefile.common

LIB_NAME	= allojit
INC_DIR		= ../../include/$(LIB_NAME)/
SRC_DIR		= ./
VPATH		= $(SRC_DIR)

SRCS		:= $(SRC_DIR)/al_Compiler.cpp
OBJS		:= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPPFLAGS	+= -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -fno-rtti
CPPFLAGS	+= -I../../include -I../../$(PLATFORM_DIR)/include

ifeq ($(PLATFORM), macosx)

	# OSX:
	LLVMLIBDIR	= ../../$(PLATFORM_DIR)/lib32/llvm
	
	LLVMLIBNAMES	:= clang clangAnalysis clangAST clangBasic clangChecker clangCodeGen clangDriver clangFrontend clangIndex clangLex clangParse clangRewrite clangSema clangSerialization CompilerDriver EnhancedDisassembly LLVMAnalysis LLVMArchive LLVMAsmParser LLVMAsmPrinter LLVMBitReader LLVMBitWriter LLVMCBackend LLVMCBackendInfo LLVMCodeGen LLVMCore LLVMCppBackend LLVMCppBackendInfo LLVMExecutionEngine LLVMInstCombine LLVMInstrumentation LLVMInterpreter LLVMipa LLVMipo LLVMJIT LLVMLinker LLVMMC LLVMMCDisassembler LLVMMCParser LLVMpic16passes LLVMScalarOpts LLVMSelectionDAG LLVMSupport LLVMSystem LLVMTarget LLVMTransformUtils LLVMX86AsmParser LLVMX86AsmPrinter LLVMX86CodeGen LLVMX86Disassembler LLVMX86Info	
	
	LLVMLIBS = $(addprefix -l, $(LLVMLIBNAMES))
	
	AR := libtool -static -arch_only i386 -syslibroot /Developer/SDKs/MacOSX10.5.sdk -L$(LLVMLIBDIR) -o 
	
else	
	# LINUX:
	LLVMLIBDIR	= ../../$(PLATFORM_DIR)/lib$(ARCH)/llvm
	
	LLVMLIBNAMES	:= LLVMJIT EnhancedDisassembly clang LLVMX86AsmParser \
			LLVMInstrumentation LLVMX86Disassembler clangFrontendTool \
			clangCodeGen CompilerDriver clangChecker clangRewrite \
			clangFrontend clangDriver clangParse clangSerialization \
			clangFrontend clangSema clangIndex clangLex clangAnalysis \
			clangAST clangBasic LLVMpic16passes LLVMAsmParser LTO \
			LLVMLinker LLVMCBackend LLVMCBackendInfo LLVMX86CodeGen \
			LLVMX86AsmPrinter LLVMX86Info LLVMAsmPrinter LLVMMCParser \
			LLVMBitWriter LLVMipo LLVMArchive LLVMBitReader \
			LLVMSelectionDAG LLVMInterpreter LLVMExecutionEngine \
			LLVMCodeGen LLVMScalarOpts LLVMInstCombine \
			LLVMTransformUtils LLVMipa LLVMAnalysis LLVMTarget \
			LLVMCore LLVMMC LLVMSupport LLVMSystem
	LLVMLIBS = $(addprefix $(LLVMLIBDIR)/lib, $(addsuffix .a, $(LLVMLIBNAMES)))
	
	# currently not working. Not sure why.
	#AR := libtool --tag=CXX --mode=link g++ -export-dynamic -static -o 
	
endif



STATIC_LIB_FLAGS += $(LLVMLIBS)

include ../../Makefile.rules

# Install library into path specified by DESTDIR
# Include files are copied into DESTDIR/include/ and
# library files are copied to DESTDIR/lib/
install: $(SLIB_PATH)	
	@echo built for $(PLATFORM) $(ARCH)
	@$(INSTALL) -d $(DESTDIR)/lib
	@$(INSTALL) -d $(DESTDIR)/include/$(LIB_NAME)
	@$(INSTALL) -C -m 644 $(INC_DIR)$/*.h* $(DESTDIR)/include/$(LIB_NAME)
	@$(INSTALL) -C -m 644 $(SLIB_PATH) $(DESTDIR)/lib
	@for v in `cd $(LLVMLIBDIR)/clang/2.8 && find * -type d ! -path '*.*'`; do \
		$(INSTALL) -d $(DESTDIR)/lib/llvm/clang/2.8/$$v; \
		$(INSTALL) -C -m 644 $(LLVMLIBDIR)/clang/2.8/$$v/* $(DESTDIR)/lib/llvm/clang/2.8/$$v;\
	done	
#	@$(RANLIB) $(DESTDIR)/lib/$(SLIB_FILE)
