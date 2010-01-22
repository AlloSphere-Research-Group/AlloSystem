#=========================================================================
# AlloCore main makefile
#=========================================================================

include ./Makefile.config

.PHONY: clean
clean:
	@cd $(IO_DIR) && make clean
	@cd $(PRO_DIR) && make clean
	@cd $(SYS_DIR) && make clean

tests:
#	@cd $(IO_DIR) && make tests
	@cd $(PRO_DIR) && make tests
	@cd $(SYS_DIR) && make tests
