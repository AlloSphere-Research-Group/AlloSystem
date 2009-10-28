#ifdef COMMONSTART
__COMMONSTART = DIAG_START_COMMON,
#undef COMMONSTART
#endif
DIAG(err_expected_colon, CLASS_ERROR, diag::MAP_ERROR, "expected ':'", 0, true)
DIAG(err_expected_namespace_name, CLASS_ERROR, diag::MAP_ERROR, "expected namespace name", 0, true)
DIAG(err_invalid_storage_class_in_func_decl, CLASS_ERROR, diag::MAP_ERROR, "invalid storage class specifier in function declarator", 0, true)
DIAG(err_no_declarators, CLASS_ERROR, diag::MAP_ERROR, "declaration does not declare anything", 0, true)
DIAG(err_param_redefinition, CLASS_ERROR, diag::MAP_ERROR, "redefinition of parameter %0", 0, true)
DIAG(err_pp_I_dash_not_supported, CLASS_ERROR, diag::MAP_ERROR, "-I- not supported, please use -iquote instead", 0, true)
DIAG(ext_longlong, CLASS_EXTENSION, diag::MAP_IGNORE, "'long long' is an extension when C99 mode is not enabled", "long-long", true)
DIAG(note_also_found_decl, CLASS_NOTE, diag::MAP_FATAL, "also found", 0, true)
DIAG(note_duplicate_case_prev, CLASS_NOTE, diag::MAP_FATAL, "previous case defined here", 0, true)
DIAG(note_forward_declaration, CLASS_NOTE, diag::MAP_FATAL, "forward declaration of %0", 0, true)
DIAG(note_invalid_subexpr_in_ice, CLASS_NOTE, diag::MAP_FATAL, "subexpression not valid in an integer constant expression", 0, true)
DIAG(note_matching, CLASS_NOTE, diag::MAP_FATAL, "to match this '%0'", 0, true)
DIAG(note_previous_declaration, CLASS_NOTE, diag::MAP_FATAL, "previous declaration is here", 0, true)
DIAG(note_previous_definition, CLASS_NOTE, diag::MAP_FATAL, "previous definition is here", 0, true)
DIAG(note_previous_implicit_declaration, CLASS_NOTE, diag::MAP_FATAL, "previous implicit declaration is here", 0, true)
DIAG(note_previous_use, CLASS_NOTE, diag::MAP_FATAL, "previous use is here", 0, true)
DIAG(note_type_being_defined, CLASS_NOTE, diag::MAP_FATAL, "definition of %0 is not complete until the closing '}'", 0, true)
DIAG(note_using_decl, CLASS_NOTE, diag::MAP_FATAL, "using", 0, true)
DIAG(warn_integer_too_large, CLASS_WARNING, diag::MAP_WARNING, "integer constant is too large for its type", 0, true)
DIAG(warn_integer_too_large_for_signed, CLASS_WARNING, diag::MAP_WARNING, "integer constant is so large that it is unsigned", 0, true)
