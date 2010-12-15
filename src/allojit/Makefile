#=========================================================================
# AlloJIT main makefile
#=========================================================================

LIB_NAME	= allojit
INC_DIR		= ../../include/$(LIB_NAME)/
SRC_DIR		= ./
VPATH		= $(SRC_DIR)

SRCS		:= $(wildcard $(SRC_DIR)/*.c*)
OBJS		:= $(addsuffix .o, $(basename $(notdir $(SRCS))))

#JITLIBS		= `ls ../../$(PLATFORM_DIR)/lib32/llvm/*.a`
#JITLIBS		= $(wildcard ../../$(PLATFORM_DIR)/lib32/llvm/*.a)

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

LLVMLIBS	= $(addprefix ../../$(PLATFORM_DIR)/lib32/llvm/lib, $(addsuffix .a, $(LLVMLIBNAMES)))

CPPFLAGS	+= -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -fno-rtti
CPPFLAGS	+= -I../../include -I../../$(PLATFORM_DIR)/include

include ../../Makefile.common
include ../../Makefile.rules

STATIC_LIB_FLAGS += $(LLVMLIBS)
#$(addprefix ../../$(PLATFORM_DIR)/lib/, $(JITLIBS))

# Install library into path specified by DESTDIR
# Include files are copied into DESTDIR/include/ and
# library files are copied to DESTDIR/lib/
install: $(SLIB_PATH)
	@$(INSTALL) -d $(DESTDIR)/lib
	@$(INSTALL) -d $(DESTDIR)/include/$(LIB_NAME)
	@$(INSTALL) -C -m 644 $(INC_DIR)$/*.h* $(DESTDIR)/include/$(LIB_NAME)
	@$(INSTALL) -C -m 644 $(SLIB_PATH) $(DESTDIR)/lib
	for v in `cd ../../$(PLATFORM_DIR)/lib32/llvm/clang/2.8 && find * -type d ! -path '*.*'`; do \
		$(INSTALL) -d $(DESTDIR)/lib/llvm/clang/2.8/$$v; \
		$(INSTALL) -C -m 644 ../../$(PLATFORM_DIR)/lib32/llvm/clang/2.8/$$v/* $(DESTDIR)/lib/llvm/clang/2.8/$$v;\
	done	
#	@$(RANLIB) $(DESTDIR)/lib/$(SLIB_FILE)
