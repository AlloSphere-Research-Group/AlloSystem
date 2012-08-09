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
	@echo     allocore .... build allocore
#	@echo     allojit ..... build allocore JIT extension
	@echo     alloutil .... build allocore utilities extension
	@echo     alloGLV ..... build allocore/GLV binding
	@echo     Gamma ....... build Gamma external
	@echo     GLV ......... build GLV external

include Makefile.rules

all:
	@for v in `ls -d */ | grep allo`; do\
		$(MAKE) --no-print-directory -C $$v install DESTDIR=../$(BUILD_DIR) linkfile -j3;\
	done

allocore: FORCE
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

alloutil: FORCE allocore
#	@$(MAKE) --no-print-directory -C $@ install BUILD_DIR=../$(BUILD_DIR) DESTDIR=../$(BUILD_DIR)
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

allonect: FORCE allocore alloutil
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR)


Gamma GLV: FORCE
	@$(MAKE) --no-print-directory -C ../$@ install DESTDIR=../$(LIB_NAME)/$(BUILD_DIR) linkfile

alloGLV: FORCE allocore GLV
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR)


clean:
	@for v in `ls -d */ | grep allo`; do\
		$(MAKE) --no-print-directory -C $$v clean;\
	done

printallos:
	@for v in `ls -d */ | grep allo`; do\
		echo $$v;\
	done
