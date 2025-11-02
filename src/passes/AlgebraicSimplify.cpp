#include "AlgebraicSimplify.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>

using namespace llvm;

PreservedAnalyses AlgebraicSimplifyPass::run(Function &F, FunctionAnalysisManager &AM) {
    bool Changed = AlgebraicSimplifier::simplifyFunction(F);
    
    if (Changed) {
        errs() << "[AlgebraicSimplify] Optimized function: " << F.getName() << "\n";
        return PreservedAnalyses::none();
    }
    
    return PreservedAnalyses::all();
}

bool AlgebraicSimplifier::simplifyFunction(Function &F) {
    bool Changed = false;
    std::vector<Instruction*> ToSimplify;
    
    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            if (I.isBinaryOp()) {
                Value *Simplified = nullptr;
                
                switch (I.getOpcode()) {
                    case Instruction::Add:
                        Simplified = simplifyAdd(&I);
                        break;
                    case Instruction::Sub:
                        Simplified = simplifySub(&I);
                        break;
                    case Instruction::Mul:
                        Simplified = simplifyMul(&I);
                        break;
                    case Instruction::SDiv:
                    case Instruction::UDiv:
                        Simplified = simplifyDiv(&I);
                        break;
                    default:
                        break;
                }
                
                if (Simplified) {
                    errs() << "  Simplified: " << I << " -> " << *Simplified << "\n";
                    I.replaceAllUsesWith(Simplified);
                    ToSimplify.push_back(&I);
                    Changed = true;
                }
            }
        }
    }
    
    for (Instruction *I : ToSimplify) {
        I->eraseFromParent();
    }
    
    return Changed;
}

Value* AlgebraicSimplifier::simplifyAdd(Instruction *I) {
    Value *LHS = I->getOperand(0);
    Value *RHS = I->getOperand(1);
    
    // x + 0 = x
    if (isConstantZero(RHS)) {
        return LHS;
    }
    
    // 0 + x = x
    if (isConstantZero(LHS)) {
        return RHS;
    }
    
    return nullptr;
}

Value* AlgebraicSimplifier::simplifySub(Instruction *I) {
    Value *LHS = I->getOperand(0);
    Value *RHS = I->getOperand(1);
    
    // x - 0 = x
    if (isConstantZero(RHS)) {
        return LHS;
    }
    
    // x - x = 0
    if (LHS == RHS) {
        return ConstantInt::get(I->getType(), 0);
    }
    
    return nullptr;
}

Value* AlgebraicSimplifier::simplifyMul(Instruction *I) {
    Value *LHS = I->getOperand(0);
    Value *RHS = I->getOperand(1);
    
    // x * 0 = 0
    if (isConstantZero(RHS)) {
        return ConstantInt::get(I->getType(), 0);
    }
    
    // 0 * x = 0
    if (isConstantZero(LHS)) {
        return ConstantInt::get(I->getType(), 0);
    }
    
    // x * 1 = x
    if (isConstantOne(RHS)) {
        return LHS;
    }
    
    // 1 * x = x
    if (isConstantOne(LHS)) {
        return RHS;
    }
    
    return nullptr;
}

Value* AlgebraicSimplifier::simplifyDiv(Instruction *I) {
    Value *LHS = I->getOperand(0);
    Value *RHS = I->getOperand(1);
    
    // x / 1 = x
    if (isConstantOne(RHS)) {
        return LHS;
    }
    
    // 0 / x = 0 (if x != 0)
    if (isConstantZero(LHS) && !isConstantZero(RHS)) {
        return ConstantInt::get(I->getType(), 0);
    }
    
    // x / x = 1 (if x != 0)
    if (LHS == RHS) {
        return ConstantInt::get(I->getType(), 1);
    }
    
    return nullptr;
}

bool AlgebraicSimplifier::isConstantZero(Value *V) {
    if (auto *CI = dyn_cast<ConstantInt>(V)) {
        return CI->isZero();
    }
    return false;
}

bool AlgebraicSimplifier::isConstantOne(Value *V) {
    if (auto *CI = dyn_cast<ConstantInt>(V)) {
        return CI->isOne();
    }
    return false;
}