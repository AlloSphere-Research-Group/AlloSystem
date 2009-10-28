#ifdef ASTSTART
__ASTSTART = DIAG_START_AST,
#undef ASTSTART
#endif
DIAG(err_asm_empty_symbolic_operand_name, CLASS_ERROR, diag::MAP_ERROR, "empty symbolic operand name in inline assembly string", 0, true)
DIAG(err_asm_invalid_escape, CLASS_ERROR, diag::MAP_ERROR, "invalid %% escape in inline assembly string", 0, true)
DIAG(err_asm_invalid_operand_number, CLASS_ERROR, diag::MAP_ERROR, "invalid operand number in inline asm string", 0, true)
DIAG(err_asm_unknown_symbolic_operand_name, CLASS_ERROR, diag::MAP_ERROR, "unknown symbolic operand name in inline assembly string", 0, true)
DIAG(err_asm_unterminated_symbolic_operand_name, CLASS_ERROR, diag::MAP_ERROR, "unterminated symbolic operand name in inline assembly string", 0, true)
DIAG(note_expr_divide_by_zero, CLASS_NOTE, diag::MAP_FATAL, "division by zero", 0, true)
