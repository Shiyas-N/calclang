#include "ConstantFolding.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

PreservedAnalyses ConstantFoldingPass::run(Function &F, FunctionAnalysisManager &AM) {
    bool Changed = CustomConstantFolder::foldFunction(F);
    
    if (Changed) {
        errs() << "[ConstantFolding] Optimized function: " << F.getName() << "\n";
        return PreservedAnalyses::none();
    }
    
    return PreservedAnalyses::all();
}

bool CustomConstantFolder::foldFunction(Function &F) {
    bool Changed = false;
    
    std::vector<Instruction*> ToFold;
    
    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            if (I.isBinaryOp()) {
                if (foldBinaryOp(&I)) {
                    ToFold.push_back(&I);
                }
            }
            else if (isa<CmpInst>(&I)) {
                if (foldComparison(&I)) {
                    ToFold.push_back(&I);
                }
            }
        }
    }
    
    for (Instruction *I : ToFold) {
        Value *Folded = nullptr;
        
        if (I->isBinaryOp()) {
            Folded = foldBinaryOp(I);
        } else if (isa<CmpInst>(I)) {
            Folded = foldComparison(I);
        }
        
        if (Folded) {
            errs() << "  Folded: " << *I << " -> " << *Folded << "\n";
            I->replaceAllUsesWith(Folded);
            I->eraseFromParent();
            Changed = true;
        }
    }
    
    return Changed;
}

Value* CustomConstantFolder::foldBinaryOp(Instruction *I) {
    ConstantInt *LHS = dyn_cast<ConstantInt>(I->getOperand(0));
    ConstantInt *RHS = dyn_cast<ConstantInt>(I->getOperand(1));
    
    if (!LHS || !RHS) {
        return nullptr; 
    }
    
    APInt L = LHS->getValue();
    APInt R = RHS->getValue();
    APInt Result;
    
    switch (I->getOpcode()) {
        case Instruction::Add:
            Result = L + R;
            break;
        case Instruction::Sub:
            Result = L - R;
            break;
        case Instruction::Mul:
            Result = L * R;
            break;
        case Instruction::SDiv:
            if (R == 0) return nullptr;
            Result = L.sdiv(R);
            break;
        case Instruction::UDiv:
            if (R == 0) return nullptr;
            Result = L.udiv(R);
            break;
        case Instruction::SRem:
            if (R == 0) return nullptr;
            Result = L.srem(R);
            break;
        case Instruction::URem:
            if (R == 0) return nullptr;
            Result = L.urem(R);
            break;
        case Instruction::Shl:
            Result = L.shl(R);
            break;
        case Instruction::LShr:
            Result = L.lshr(R);
            break;
        case Instruction::AShr:
            Result = L.ashr(R);
            break;
        case Instruction::And:
            Result = L & R;
            break;
        case Instruction::Or:
            Result = L | R;
            break;
        case Instruction::Xor:
            Result = L ^ R;
            break;
        default:
            return nullptr;
    }
    
    return ConstantInt::get(I->getType(), Result);
}

Value* CustomConstantFolder::foldComparison(Instruction *I) {
    CmpInst *Cmp = cast<CmpInst>(I);
    
    ConstantInt *LHS = dyn_cast<ConstantInt>(Cmp->getOperand(0));
    ConstantInt *RHS = dyn_cast<ConstantInt>(Cmp->getOperand(1));
    
    if (!LHS || !RHS) {
        return nullptr;
    }
    
    APInt L = LHS->getValue();
    APInt R = RHS->getValue();
    bool Result = false;
    
    switch (Cmp->getPredicate()) {
        case CmpInst::ICMP_EQ:  Result = (L == R); break;
        case CmpInst::ICMP_NE:  Result = (L != R); break;
        case CmpInst::ICMP_UGT: Result = L.ugt(R); break;
        case CmpInst::ICMP_UGE: Result = L.uge(R); break;
        case CmpInst::ICMP_ULT: Result = L.ult(R); break;
        case CmpInst::ICMP_ULE: Result = L.ule(R); break;
        case CmpInst::ICMP_SGT: Result = L.sgt(R); break;
        case CmpInst::ICMP_SGE: Result = L.sge(R); break;
        case CmpInst::ICMP_SLT: Result = L.slt(R); break;
        case CmpInst::ICMP_SLE: Result = L.sle(R); break;
        default: return nullptr;
    }
    
    return ConstantInt::get(Type::getInt1Ty(I->getContext()), Result);
}