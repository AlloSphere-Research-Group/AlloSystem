
#ifdef GET_DIAG_ARRAYS
static const char DiagSubGroup0[] = { 18, -1 };
static const short DiagArray1[] = { diag::pp_hash_warning, -1 };
static const char DiagSubGroup3[] = { 47, 55, -1 };
static const short DiagArray9[] = { diag::escaped_newline_block_comment_end, diag::ext_bcpl_comment, diag::ext_multi_line_bcpl_comment, diag::warn_nested_block_comment, -1 };
static const short DiagArray12[] = { diag::warn_deprecated, -1 };
static const short DiagArray16[] = { diag::warn_empty_if_body, -1 };
static const char DiagSubGroup17[] = { 19, -1 };
static const char DiagSubGroup18[] = { 92, -1 };
static const short DiagArray19[] = { diag::ext_pp_extra_tokens_at_eol, -1 };
static const short DiagArray20[] = { diag::warn_floatingpoint_eq, -1 };
static const short DiagArray21[] = { diag::warn_printf_asterisk_precision_wrong_type, diag::warn_printf_asterisk_width_wrong_type, diag::warn_printf_format_string_contains_null_char, diag::warn_printf_format_string_is_wide_literal, diag::warn_printf_insufficient_data_args, diag::warn_printf_invalid_conversion, diag::warn_printf_missing_format_string, -1 };
static const char DiagSubGroup21[] = { 22, 26, 51, -1 };
static const short DiagArray22[] = { diag::warn_printf_too_many_data_args, -1 };
static const short DiagArray23[] = { diag::warn_printf_nonliteral, -1 };
static const char DiagSubGroup23[] = { 24, -1 };
static const short DiagArray24[] = { diag::warn_printf_nonliteral_noargs, diag::warn_printf_write_back, -1 };
static const char DiagSubGroup24[] = { 21, -1 };
static const char DiagSubGroup25[] = { 21, -1 };
static const short DiagArray26[] = { diag::warn_printf_empty_format_string, -1 };
static const char DiagSubGroup27[] = { 23, 24, 25, -1 };
static const short DiagArray28[] = { diag::ext_four_char_character_literal, -1 };
static const char DiagSubGroup29[] = { 30, 31, -1 };
static const short DiagArray30[] = { diag::ext_implicit_function_decl, diag::warn_implicit_function_decl, -1 };
static const short DiagArray31[] = { diag::ext_missing_type_specifier, -1 };
static const short DiagArray36[] = { diag::warn_falloff_noreturn_function, diag::warn_noreturn_function_has_return_expr, -1 };
static const short DiagArray37[] = { diag::warn_offsetof_non_pod_type, -1 };
static const short DiagArray39[] = { diag::ext_longlong, -1 };
static const short DiagArray40[] = { diag::warn_struct_class_tag_mismatch, -1 };
static const short DiagArray45[] = { diag::warn_suggest_noreturn_block, diag::warn_suggest_noreturn_function, -1 };
static const short DiagArray46[] = { diag::warn_missing_prototype, -1 };
static const char DiagSubGroup47[] = { 9, 21, 29, 40, 48, 62, 77, 78, 84, 85, 93, 94, 96, 97, 59, 61, 82, -1 };
static const short DiagArray48[] = { diag::ext_multichar_character_literal, -1 };
static const short DiagArray51[] = { diag::warn_null_arg, -1 };
static const short DiagArray57[] = { diag::ext_typecheck_convert_incompatible_pointer_sign, -1 };
static const short DiagArray59[] = { diag::warn_objc_property_attr_mutually_exclusive, -1 };
static const short DiagArray61[] = { diag::warn_base_initialized, diag::warn_field_initialized, -1 };
static const short DiagArray62[] = { diag::ext_return_has_expr, diag::ext_return_missing_expr, diag::warn_falloff_nonvoid_function, diag::warn_maybe_falloff_nonvoid_function, diag::warn_return_missing_expr, -1 };
static const short DiagArray78[] = { diag::trigraph_converted, diag::trigraph_ends_block_comment, diag::trigraph_ignored, diag::trigraph_ignored_block_comment, -1 };
static const short DiagArray80[] = { diag::warn_redefinition_of_typedef, -1 };
static const short DiagArray81[] = { diag::warn_unavailable, -1 };
static const short DiagArray82[] = { diag::warn_undeclared_selector, -1 };
static const short DiagArray83[] = { diag::warn_pp_undef_identifier, -1 };
static const short DiagArray85[] = { diag::ext_stdc_pragma_ignored, diag::ext_stdc_pragma_syntax, diag::ext_stdc_pragma_syntax_eom, diag::warn_pragma_diagnostic_clang_cannot_ppp, diag::warn_pragma_diagnostic_clang_invalid, diag::warn_pragma_diagnostic_gcc_invalid, diag::warn_pragma_diagnostic_invalid_option, diag::warn_pragma_diagnostic_invalid_token, diag::warn_pragma_diagnostic_unknown_warning, diag::warn_pragma_ignored, diag::warn_stdc_fenv_access_not_supported, -1 };
static const short DiagArray86[] = { diag::warn_unknown_warning_option, -1 };
static const char DiagSubGroup87[] = { 88, 89, 90, 92, 93, 94, -1 };
static const short DiagArray91[] = { diag::pp_macro_not_used, -1 };
static const short DiagArray92[] = { diag::warn_unused_parameter, -1 };
static const short DiagArray93[] = { diag::warn_unused_expr, diag::warn_unused_property_expr, -1 };
static const short DiagArray96[] = { diag::warn_incompatible_vectors, -1 };
#endif // GET_DIAG_ARRAYS


#ifdef GET_DIAG_TABLE
  { "",                              0, DiagSubGroup0 },
  { "#warnings",                     DiagArray1, 0 },
  { "aggregate-return",              0, 0 },
  { "all",                           0, DiagSubGroup3 },
  { "bad-function-cast",             0, 0 },
  { "cast-align",                    0, 0 },
  { "cast-qual",                     0, 0 },
  { "char-align",                    0, 0 },
  { "char-subscripts",               0, 0 },
  { "comment",                       DiagArray9, 0 },
  { "conversion",                    0, 0 },
  { "declaration-after-statement",   0, 0 },
  { "deprecated-declarations",       DiagArray12, 0 },
  { "disabled-optimization",         0, 0 },
  { "discard-qual",                  0, 0 },
  { "div-by-zero",                   0, 0 },
  { "empty-body",                    DiagArray16, 0 },
  { "endif-labels",                  0, DiagSubGroup17 },
  { "extra",                         0, DiagSubGroup18 },
  { "extra-tokens",                  DiagArray19, 0 },
  { "float-equal",                   DiagArray20, 0 },
  { "format",                        DiagArray21, DiagSubGroup21 },
  { "format-extra-args",             DiagArray22, 0 },
  { "format-nonliteral",             DiagArray23, DiagSubGroup23 },
  { "format-security",               DiagArray24, DiagSubGroup24 },
  { "format-y2k",                    0, DiagSubGroup25 },
  { "format-zero-length",            DiagArray26, 0 },
  { "format=2",                      0, DiagSubGroup27 },
  { "four-char-constants",           DiagArray28, 0 },
  { "implicit",                      0, DiagSubGroup29 },
  { "implicit-function-declaration", DiagArray30, 0 },
  { "implicit-int",                  DiagArray31, 0 },
  { "import",                        0, 0 },
  { "init-self",                     0, 0 },
  { "inline",                        0, 0 },
  { "int-to-pointer-cast",           0, 0 },
  { "invalid-noreturn",              DiagArray36, 0 },
  { "invalid-offsetof",              DiagArray37, 0 },
  { "invalid-pch",                   0, 0 },
  { "long-long",                     DiagArray39, 0 },
  { "mismatched-tags",               DiagArray40, 0 },
  { "missing-braces",                0, 0 },
  { "missing-declarations",          0, 0 },
  { "missing-field-initializers",    0, 0 },
  { "missing-format-attribute",      0, 0 },
  { "missing-noreturn",              DiagArray45, 0 },
  { "missing-prototypes",            DiagArray46, 0 },
  { "most",                          0, DiagSubGroup47 },
  { "multichar",                     DiagArray48, 0 },
  { "nested-externs",                0, 0 },
  { "newline-eof",                   0, 0 },
  { "nonnull",                       DiagArray51, 0 },
  { "nonportable-cfstrings",         0, 0 },
  { "old-style-definition",          0, 0 },
  { "packed",                        0, 0 },
  { "parentheses",                   0, 0 },
  { "pointer-arith",                 0, 0 },
  { "pointer-sign",                  DiagArray57, 0 },
  { "pointer-to-int-cast",           0, 0 },
  { "readonly-setter-attrs",         DiagArray59, 0 },
  { "redundant-decls",               0, 0 },
  { "reorder",                       DiagArray61, 0 },
  { "return-type",                   DiagArray62, 0 },
  { "sequence-point",                0, 0 },
  { "shadow",                        0, 0 },
  { "shorten-64-to-32",              0, 0 },
  { "sign-compare",                  0, 0 },
  { "strict-aliasing",               0, 0 },
  { "strict-aliasing=0",             0, 0 },
  { "strict-aliasing=1",             0, 0 },
  { "strict-aliasing=2",             0, 0 },
  { "strict-overflow",               0, 0 },
  { "strict-overflow=0",             0, 0 },
  { "strict-overflow=1",             0, 0 },
  { "strict-overflow=2",             0, 0 },
  { "strict-prototypes",             0, 0 },
  { "strict-selector-match",         0, 0 },
  { "switch",                        0, 0 },
  { "trigraphs",                     DiagArray78, 0 },
  { "type-limits",                   0, 0 },
  { "typedef-redefinition",          DiagArray80, 0 },
  { "unavailable-declarations",      DiagArray81, 0 },
  { "undeclared-selector",           DiagArray82, 0 },
  { "undef",                         DiagArray83, 0 },
  { "uninitialized",                 0, 0 },
  { "unknown-pragmas",               DiagArray85, 0 },
  { "unknown-warning-option",        DiagArray86, 0 },
  { "unused",                        0, DiagSubGroup87 },
  { "unused-argument",               0, 0 },
  { "unused-function",               0, 0 },
  { "unused-label",                  0, 0 },
  { "unused-macros",                 DiagArray91, 0 },
  { "unused-parameter",              DiagArray92, 0 },
  { "unused-value",                  DiagArray93, 0 },
  { "unused-variable",               0, 0 },
  { "variadic-macros",               0, 0 },
  { "vector-conversions",            DiagArray96, 0 },
  { "volatile-register-var",         0, 0 },
  { "write-strings",                 0, 0 },
#endif // GET_DIAG_TABLE

