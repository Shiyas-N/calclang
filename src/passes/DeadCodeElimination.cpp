#include "DeadCodeElimination.h"
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>

using namespace llvm;

PreservedAnalyses DeadCodeEliminationPass::run(Function &F, FunctionAnalysisManager &AM) {
    bool Changed = DeadCodeEliminator::eliminateDeadCode(F);
    
    if (Changed) {
        errs() << "[DeadCodeElimination] Optimized function: " << F.getName() << "\n";
        return PreservedAnalyses::none();
    }
    
    return PreservedAnalyses::all();
}

bool DeadCodeEliminator::eliminateDeadCode(Function &F) {
    bool Changed = false;
    bool LocalChanged;
    
    do {
        LocalChanged = false;
        std::vector<Instruction*> ToDelete;
        
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if (isInstructionDead(&I)) {
                    ToDelete.push_back(&I);
                }
            }
        }
        
        for (Instruction *I : ToDelete) {
            errs() << "  Removing dead instruction: " << *I << "\n";
            I->eraseFromParent();
            LocalChanged = true;
            Changed = true;
        }
        
    } while (LocalChanged);
    
    return Changed;
}

bool DeadCodeEliminator::isInstructionDead(Instruction *I) {
    if (I->isTerminator()) {
        return false;
    }
    
    if (hasSideEffects(I)) {
        return false;
    }
    
    return I->use_empty();
}

bool DeadCodeEliminator::hasSideEffects(Instruction *I) {
    if (isa<StoreInst>(I)) {
        return true;
    }
    
    if (auto *Call = dyn_cast<CallInst>(I)) {
        Function *Callee = Call->getCalledFunction();
        if (!Callee) return true; 
        
        StringRef Name = Callee->getName();
        if (Name == "llvm.dbg.declare" || Name == "llvm.dbg.value") {
            return false;
        }
        
        return true;
    }
    
    if (isa<FenceInst>(I) || I->isAtomic()) {
        return true;
    }
    
    if (isa<AllocaInst>(I)) {
        return true;
    }
    
    if (auto *Load = dyn_cast<LoadInst>(I)) {
        return Load->isVolatile();
    }
    
    return false;
}