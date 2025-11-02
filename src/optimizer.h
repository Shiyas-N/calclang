#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <llvm/IR/Module.h>
#include <string>

class Optimizer {
public:
    Optimizer(llvm::Module *M);
    
    void optimize();
    
    void optimizeLevel(int level);
    
    void runConstantFolding();
    void runDeadCodeElimination();
    void runAlgebraicSimplify();
    
    void printStatistics();
    
private:
    llvm::Module *module;
    
    struct Stats {
        int instructionsBefore = 0;
        int instructionsAfter = 0;
        int constantsFolded = 0;
        int deadCodeRemoved = 0;
        int algebraicSimplifications = 0;
    } stats;
    
    void countInstructions(int &count);
};

#endif