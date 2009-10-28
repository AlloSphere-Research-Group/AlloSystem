//===-- llvm/MC/MCValue.h - MCValue class -----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the MCValue class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_MC_MCVALUE_H
#define LLVM_MC_MCVALUE_H

#include "llvm/Support/DataTypes.h"
#include "llvm/MC/MCSymbol.h"
#include <cassert>

namespace llvm {
class MCSymbol;
class raw_ostream;

/// MCValue - This represents an "assembler immediate".  In its most general
/// form, this can hold "SymbolA - SymbolB + imm64".  Not all targets supports
/// relocations of this general form, but we need to represent this anyway.
///
/// In the general form, SymbolB can only be defined if SymbolA is, and both
/// must be in the same (non-external) section. The latter constraint is not
/// enforced, since a symbol's section may not be known at construction.
///
/// Note that this class must remain a simple POD value class, because we need
/// it to live in unions etc.
class MCValue {
  MCSymbol *SymA, *SymB;
  int64_t Cst;
public:

  int64_t getConstant() const { return Cst; }
  MCSymbol *getSymA() const { return SymA; }
  MCSymbol *getSymB() const { return SymB; }

  /// isAbsolute - Is this an absolute (as opposed to relocatable) value.
  bool isAbsolute() const { return !SymA && !SymB; }

  /// getAssociatedSection - For relocatable values, return the section the
  /// value is associated with.
  ///
  /// @result - The value's associated section, or null for external or constant
  /// values.
  const MCSection *getAssociatedSection() const {
    return SymA ? SymA->getSection() : 0;
  }

  /// print - Print the value to the stream \arg OS.
  void print(raw_ostream &OS) const;
  
  /// dump - Print the value to stderr.
  void dump() const;

  static MCValue get(MCSymbol *SymA, MCSymbol *SymB = 0, int64_t Val = 0) {
    MCValue R;
    assert((!SymB || SymA) && "Invalid relocatable MCValue!");
    R.Cst = Val;
    R.SymA = SymA;
    R.SymB = SymB;
    return R;
  }
  
  static MCValue get(int64_t Val) {
    MCValue R;
    R.Cst = Val;
    R.SymA = 0;
    R.SymB = 0;
    return R;
  }
  
};

} // end namespace llvm

#endif
