#ifndef COSTANT_FOLDING_H
#define COSTANT_FOLDING_H

#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/Pass.h>

using namespace llvm;

class ConstantFoldingPass : public PassInfoMixin<ConstantFoldingPass>{
    public:
        PreservedAnalyses run(Function &F,FunctionAnalysisManager &AM);

        static StringRef name(){
            return "ConstantFoldingPass";
        }
};

class CustomConstantFolder{
    public:
        static bool foldFunction(Function &F);
    
    private:
        static Value* foldBinaryOp(Instruction *I);
        static Value* foldComparison(Instruction *I);
        // static Value* foldBinaryOp(BinaryOperator *BO);
        // static Value* foldComparison(CmpInst *Cmp);
};


#endif