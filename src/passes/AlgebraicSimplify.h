#ifndef ALGEBRAIC_SIMPLIFY_H
#define ALGEBRAIC_SIMPLIFY_H

#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstrTypes.h>

using namespace llvm;

class AlgebraicSimplifyPass : public PassInfoMixin<AlgebraicSimplifyPass> {
public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
    
    static StringRef name() { return "AlgebraicSimplifyPass"; }
};

class AlgebraicSimplifier {
public:
    static bool simplifyFunction(Function &F);
    
private:
    static Value* simplifyAdd(Instruction *I);
    static Value* simplifySub(Instruction *I);
    static Value* simplifyMul(Instruction *I);
    static Value* simplifyDiv(Instruction *I);
    
    static bool isConstantZero(Value *V);
    static bool isConstantOne(Value *V);
};

#endif