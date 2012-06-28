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
#	@echo     Gamma ....... build Gamma external
#	@echo     GLV ......... build GLV external

include Makefile.rules

allocore: FORCE
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR)
#	@$(MAKE) --no-print-directory -C $@ external

alloutil: FORCE allocore
#	@$(MAKE) --no-print-directory -C $@ install BUILD_DIR=../$(BUILD_DIR) DESTDIR=../$(BUILD_DIR)
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR)
#	@$(MAKE) --no-print-directory -C $@ external

Gamma GLV: FORCE
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR)

alloGLV: FORCE allocore GLV
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR)

