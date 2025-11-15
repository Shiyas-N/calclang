#ifndef JIT_H
#define JIT_H

#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <memory>

class JITCompiler {
public:
    JITCompiler(llvm::Module *M);
    ~JITCompiler();
    
    bool initialize();
    
    int executeMain();
    
private:
    llvm::Module *module;
    std::unique_ptr<llvm::orc::LLJIT> jit;
};

#endif