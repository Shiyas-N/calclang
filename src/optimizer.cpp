#include "optimizer.h"
#include "passes/ConstantFolding.h"
#include "passes/DeadCodeElimination.h"
#include "passes/AlgebraicSimplify.h"

#include <llvm/IR/Function.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/IR/PassInstrumentation.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

Optimizer::Optimizer(Module *M) : module(M) {
    countInstructions(stats.instructionsBefore);
}

void Optimizer::optimize() {
    errs() << "\n=== Starting Optimization ===\n";
    errs() << "Initial instruction count: " << stats.instructionsBefore << "\n\n";
    
    for (int i = 0; i < 3; i++) {
        errs() << "--- Pass iteration " << (i + 1) << " ---\n";
        
        runConstantFolding();
        runAlgebraicSimplify();
        runDeadCodeElimination();
        
        errs() << "\n";
    }
    
    countInstructions(stats.instructionsAfter);
    printStatistics();
}

void Optimizer::optimizeLevel(int level) {
    errs() << "\n=== Optimization Level -O" << level << " ===\n";
    
    switch (level) {
        case 0:
            errs() << "No optimization applied\n";
            break;
            
        case 1:
            runConstantFolding();
            runDeadCodeElimination();
            break;
            
        case 2:
            runConstantFolding();
            runAlgebraicSimplify();
            runDeadCodeElimination();
            runConstantFolding();
            runDeadCodeElimination();
            break;
            
        case 3:
            optimize();
            break;
            
        default:
            errs() << "Unknown optimization level\n";
            break;
    }
    
    countInstructions(stats.instructionsAfter);
    printStatistics();
}

void Optimizer::runConstantFolding() {
    errs() << "[Running] Constant Folding Pass\n";
    
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    ModuleAnalysisManager MAM;
    
    PassBuilder PB;
    PB.registerModuleAnalyses(MAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    
    FAM.registerPass([&] { return PassInstrumentationAnalysis(); });
    
    ConstantFoldingPass CFP;
    
    for (Function &F : *module) {
        if (!F.isDeclaration()) {
            CFP.run(F, FAM);
        }
    }
}

void Optimizer::runDeadCodeElimination() {
    errs() << "[Running] Dead Code Elimination Pass\n";
    
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    ModuleAnalysisManager MAM;
    
    PassBuilder PB;
    PB.registerModuleAnalyses(MAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    
    FAM.registerPass([&] { return PassInstrumentationAnalysis(); });
    
    DeadCodeEliminationPass DCEP;
    
    for (Function &F : *module) {
        if (!F.isDeclaration()) {
            DCEP.run(F, FAM);
        }
    }
}

void Optimizer::runAlgebraicSimplify() {
    errs() << "[Running] Algebraic Simplification Pass\n";
    
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    ModuleAnalysisManager MAM;
    
    PassBuilder PB;
    PB.registerModuleAnalyses(MAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    
    FAM.registerPass([&] { return PassInstrumentationAnalysis(); });
    
    AlgebraicSimplifyPass ASP;
    
    for (Function &F : *module) {
        if (!F.isDeclaration()) {
            ASP.run(F, FAM);
        }
    }
}

void Optimizer::countInstructions(int &count) {
    count = 0;
    for (Function &F : *module) {
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                count++;
            }
        }
    }
}

void Optimizer::printStatistics() {
    errs() << "\n=== Optimization Statistics ===\n";
    errs() << "Instructions before: " << stats.instructionsBefore << "\n";
    errs() << "Instructions after:  " << stats.instructionsAfter << "\n";
    
    int removed = stats.instructionsBefore - stats.instructionsAfter;
    if (stats.instructionsBefore > 0) {
        float percent = (removed * 100.0f) / stats.instructionsBefore;
        errs() << "Instructions removed: " << removed << " (" << percent << "%)\n";
    }
    
    errs() << "===============================\n\n";
}