#=========================================================================
# AlloSystem main Makefile
#=========================================================================

LIB_NAME = allosystem

include Makefile.common

# List of all the module directories
MODULE_DIRS := $(shell ls -d */ | grep allo)

#--------------------------------------------------------------------------
# Rules
#--------------------------------------------------------------------------

help:
	@echo No rule specified.
	@echo The possible rules are:
	@echo "    all .............. build all modules found in this directory"
	@echo "    allocore ......... build allocore"
	@echo "    alloutil ......... build allocore utilities extension"
	@echo "    allocv ........... build allocore/OpenCV binding"
	@echo "    alloGLV .......... build allocore/GLV binding"
	@echo "    allonect ......... build allocore/freenect binding"
	@echo "    allovsr .......... build allocore/vsr binding"
	@echo "    Gamma ............ build Gamma external"
	@echo "    GLV .............. build GLV external"
	@echo "    vsr .............. build vsr external [use with optional INSTALL_VSR_PCH flag set to 1 or 0]"
	@echo "    clean ............ clean all modules found in this directory"
	@echo "    docs ............. open API documentation"
	@echo "    gatherexamples ... create examples/ directory with symlinks to module examples"


all:
	@for v in $(MODULE_DIRS); do\
		$(MAKE) --no-print-directory -C $$v install DESTDIR=../$(BUILD_DIR) linkfile;\
	done

allocore: FORCE
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

alloutil: FORCE allocore
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

allocv: FORCE allocore
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

Gamma GLV: FORCE
	@$(MAKE) --no-print-directory -C ../$@ install DESTDIR=$(CURDIR)/$(BUILD_DIR) linkfile

alloGLV: FORCE allocore GLV
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

allonect: FORCE allocore alloutil
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR)

#Default Installs Precompiled Header -- set to 0 to avoid this copy
INSTALL_VSR_PCH = 1
vsr: FORCE
	@echo "compiling versor library and installing to AlloSystem/build directory"
	@$(MAKE) --no-print-directory -C ../$@ install INSTALL_PCH=$(INSTALL_VSR_PCH) DESTDIR=$(CURDIR)/$(BUILD_DIR) linkfile

allovsr: FORCE allocore
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR) linkfile


clean:
	@for v in $(MODULE_DIRS); do\
		$(MAKE) --no-print-directory -C $$v clean;\
	done


# Create/view API documentation
DOC_INDEX_FILE := doc/www/doxy/html/index.html
$(DOC_INDEX_FILE): doc/Doxyfile allocore/allocore/*
	@if [ `which doxygen` ]; then \
		cd doc && doxygen Doxyfile && cd ..;\
	elif [ `which /Applications/Doxygen.app/Contents/Resources/doxygen` ]; then \
		cd doc && /Applications/Doxygen.app/Contents/Resources/doxygen Doxyfile && cd ..;\
	else \
		echo "Error: doxygen not found.";\
		echo "doxygen is required to create the documentation.";\
		printf "Please install it using ";\
		if [ `which apt-get` ]; then printf "\"sudo apt-get install doxygen\"";\
		elif [ `which port` ]; then printf "\"sudo port install doxygen\"";\
		elif [ `which brew` ]; then printf "\"brew install doxygen\"";\
		else printf "a package manager, e.g., apt-get (Linux), MacPorts or Homebrew (Mac OSX),";\
		fi;\
		printf " and try again. You can also create the documentation manually \
by downloading doxygen from www.doxygen.org and running it on the file $<.\n";\
		exit 127;\
	fi

docs: $(DOC_INDEX_FILE)
ifeq ($(PLATFORM), linux)
	@xdg-open $< &
else ifeq ($(PLATFORM), macosx)
	@open $<
endif


# This attempts to determine what modules have been built by looking in the 
# build directory.
BUILT_MODULES := $(basename $(shell ls $(BUILD_DIR)/lib/ 2> /dev/null))
BUILT_MODULES := $(subst lib,,$(BUILT_MODULES))
BUILT_MODULES := $(filter allo%, $(BUILT_MODULES))

# This is a hack to ensure that Gamma and GLV linker dependencies are included.
# A better, more general solution would involve having a user dependency 
# directory that is scanned.
ifneq ($(shell ls $(BUILD_DIR)/lib/ 2> /dev/null | grep Gamma),)
	BUILT_MODULES += ../Gamma/
endif
ifneq ($(shell ls $(BUILD_DIR)/lib/ 2> /dev/null | grep GLV),)
	BUILT_MODULES += ../GLV/
endif

-include $(addsuffix /Makefile.link, $(BUILT_MODULES))

LDFLAGS += -L$(BUILD_DIR)/lib/
CPPFLAGS += -I$(BUILD_DIR)/include/

include Makefile.rules

include Makefile.buildandrun
