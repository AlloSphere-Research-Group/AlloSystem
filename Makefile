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
	@echo "    alloGLV .......... build allocore/GLV binding"
	@echo "    allovsr .......... build allocore/vsr binding"
	@echo "    Gamma ............ build Gamma external"
	@echo "    GLV .............. build GLV external"
	@echo "    vsr .............. build vsr external [use with optional INSTALL_VSR_PCH flag set to 1 or 0]"
	@echo "    clean ............ clean all modules found in this directory"
	@echo "    gatherexamples ... create examples/ directory with symlinks to module examples"

include Makefile.rules

all:
	@for v in $(MODULE_DIRS); do\
		$(MAKE) --no-print-directory -C $$v install DESTDIR=../$(BUILD_DIR) linkfile;\
	done

allocore: FORCE
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

alloutil: FORCE allocore
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

allonect: FORCE allocore alloutil
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR)

Gamma GLV: FORCE
	@$(MAKE) --no-print-directory -C ../$@ install DESTDIR=$(CURDIR)/$(BUILD_DIR) linkfile

alloGLV: FORCE allocore GLV
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

#Default Installs Precompiled Header -- set to 0 to avoid this copy
INSTALL_VSR_PCH = 1
vsr: FORCE
	@echo "compiling versor library and installing to AlloSystem/build directory"
	@$(MAKE) --no-print-directory -C ../$@ install INSTALL_PCH=$(INSTALL_VSR_PCH) DESTDIR=$(CURDIR)/$(BUILD_DIR) linkfile

allovsr: FORCE
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR)


clean:
	@for v in $(MODULE_DIRS); do\
		$(MAKE) --no-print-directory -C $$v clean;\
	done

# Create symlinks to all examples/ directories found in modules
gatherexamples:
	@install -d examples
	@for v in $(subst /,,$(MODULE_DIRS)); do\
		if [ -d $$v/examples/ ]; then\
			cd examples/;\
			ln -s ../$$v/examples/ $$v;\
			cd ..;\
		fi;\
	done


# This attempts to determine what modules have been built by looking in the build dir
#BUILT_MODULES := $(basename $(shell if [ -d $(BUILD_DIR)/lib/ ]; then ls $(BUILD_DIR)/lib/; fi))
BUILT_MODULES := $(basename $(shell ls $(BUILD_DIR)/lib/ 2> /dev/null))
BUILT_MODULES := $(subst lib,,$(BUILT_MODULES))
BUILT_MODULES := $(filter allo%, $(BUILT_MODULES))

# This is a hack to ensure that Gamma and GLV linker dependencies are included.
# A better, more general solution would involve having a user dependency directory that is scanned.
ifneq ($(shell ls $(BUILD_DIR)/lib/ 2> /dev/null | grep Gamma),)
	BUILT_MODULES += ../Gamma/
endif
ifneq ($(shell ls $(BUILD_DIR)/lib/ 2> /dev/null | grep GLV),)
	BUILT_MODULES += ../GLV/
endif

-include $(addsuffix /Makefile.link, $(BUILT_MODULES))
LDFLAGS += -L$(BUILD_DIR)/lib/
include Makefile.buildandrun
