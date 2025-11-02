#ifndef DEAD_CODE_ELIMINATION_H
#define DEAD_CODE_ELIMINATION_H

#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>

using namespace llvm;

class DeadCodeEliminationPass : public PassInfoMixin<DeadCodeEliminationPass> {
public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
    
    static StringRef name() { return "DeadCodeEliminationPass"; }
};

class DeadCodeEliminator {
public:
    static bool eliminateDeadCode(Function &F);
    
private:
    static bool isInstructionDead(Instruction *I);
    static bool hasSideEffects(Instruction *I);
};

#endif