//===- llvm/Analysis/ProfileInfo.h - Profile Info Interface -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the generic ProfileInfo interface, which is used as the
// common interface used by all clients of profiling information, and
// implemented either by making static guestimations, or by actually reading in
// profiling information gathered by running the program.
//
// Note that to be useful, all profile-based optimizations should preserve
// ProfileInfo, which requires that they notify it when changes to the CFG are
// made. (This is not implemented yet.)
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_PROFILEINFO_H
#define LLVM_ANALYSIS_PROFILEINFO_H

#include "llvm/BasicBlock.h"
#include <cassert>
#include <string>
#include <map>

namespace llvm {
  class Function;
  class Pass;

  /// ProfileInfo Class - This class holds and maintains profiling
  /// information for some unit of code.
  class ProfileInfo {
  public:
    // Types for handling profiling information.
    typedef std::pair<const BasicBlock*, const BasicBlock*> Edge;
    typedef std::map<Edge, double> EdgeCounts;
    typedef std::map<const BasicBlock*, double> BlockCounts;

  protected:
    // EdgeCounts - Count the number of times a transition between two blocks is
    // executed.  As a special case, we also hold an edge from the null
    // BasicBlock to the entry block to indicate how many times the function was
    // entered.
    std::map<const Function*, EdgeCounts> EdgeInformation;

    // BlockCounts - Count the number of times a block is executed.
    std::map<const Function*, BlockCounts> BlockInformation;

    // FunctionCounts - Count the number of times a function is executed.
    std::map<const Function*, double> FunctionInformation;
  public:
    static char ID; // Class identification, replacement for typeinfo
    virtual ~ProfileInfo();  // We want to be subclassed

    // MissingValue - The value that is returned for execution counts in case
    // no value is available.
    static const int MissingValue = -1;

    // getFunction() - Returns the Function for an Edge, checking for validity.
    static const Function* getFunction(Edge e) {
      assert(e.second && "Invalid ProfileInfo::Edge");
      return e.second->getParent();
    }

    // getEdge() - Creates an Edge from two BasicBlocks.
    static Edge getEdge(const BasicBlock* Src, const BasicBlock* Dest) {
      return std::make_pair(Src, Dest);
    }

    //===------------------------------------------------------------------===//
    /// Profile Information Queries
    ///
    double getExecutionCount(const Function *F);

    double getExecutionCount(const BasicBlock *BB);

    double getEdgeWeight(Edge e) const {
      std::map<const Function*, EdgeCounts>::const_iterator J =
        EdgeInformation.find(getFunction(e));
      if (J == EdgeInformation.end()) return MissingValue;

      EdgeCounts::const_iterator I = J->second.find(e);
      if (I == J->second.end()) return MissingValue;

      return I->second;
    }

    //===------------------------------------------------------------------===//
    /// Analysis Update Methods
    ///

  };

  /// createProfileLoaderPass - This function returns a Pass that loads the
  /// profiling information for the module from the specified filename, making
  /// it available to the optimizers.
  Pass *createProfileLoaderPass(const std::string &Filename);
} // End llvm namespace

#endif
