#=========================================================================
# Allos main makefile
#=========================================================================

LIB_NAME = allos

include Makefile.common

#--------------------------------------------------------------------------
# Rules
#--------------------------------------------------------------------------

help:
	@echo No rule specified.
	@echo The possible rules are:
	@echo "    all ......... build all modules found in this directory"
	@echo "    allocore .... build allocore"
	@echo "    alloutil .... build allocore utilities extension"
	@echo "    alloGLV ..... build allocore/GLV binding"
	@echo "    Gamma ....... build Gamma external"
	@echo "    GLV ......... build GLV external"
	@echo "    clean........ clean all modules found in this directory"

include Makefile.rules

all:
	@for v in `ls -d */ | grep allo`; do\
		$(MAKE) --no-print-directory -j3 -C $$v install DESTDIR=../$(BUILD_DIR) linkfile;\
	done

allocore: FORCE
	@$(MAKE) --no-print-directory -j3 -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

alloutil: FORCE allocore
#	@$(MAKE) --no-print-directory -j3 -C $@ install BUILD_DIR=../$(BUILD_DIR) DESTDIR=../$(BUILD_DIR)
	@$(MAKE) --no-print-directory -j3 -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

allonect: FORCE allocore alloutil
	@$(MAKE) --no-print-directory -j3 -C $@ install DESTDIR=../$(BUILD_DIR)


Gamma GLV: FORCE
	@$(MAKE) --no-print-directory -j3 -C ../$@ install DESTDIR=../$(LIB_NAME)/$(BUILD_DIR) linkfile

alloGLV: FORCE allocore GLV
	@$(MAKE) --no-print-directory -j3 -C $@ install DESTDIR=../$(BUILD_DIR)


clean:
	@for v in `ls -d */ | grep allo`; do\
		$(MAKE) --no-print-directory -C $$v clean;\
	done

printallos:
	@for v in `ls -d */ | grep allo`; do\
		echo $$v;\
	done


include Makefile.buildandrun
