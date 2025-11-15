#include "jit.h"
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Support/Error.h>

using namespace llvm;
using namespace llvm::orc;


JITCompiler::JITCompiler(Module *M) : module(M), jit(nullptr) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
}

JITCompiler::~JITCompiler() {
}

bool JITCompiler::initialize() {
    auto JITOrErr = LLJITBuilder().create();
    
    if (!JITOrErr) {
        errs() << "Failed to create JIT: " << toString(JITOrErr.takeError()) << "\n";
        return false;
    }
    
    jit = std::move(*JITOrErr);
    
    auto ClonedModule = CloneModule(*module);
    auto TSCtx = std::make_unique<LLVMContext>();
    auto TSM = ThreadSafeModule(std::move(ClonedModule), std::move(TSCtx));
    
    if (auto Err = jit->addIRModule(std::move(TSM))) {
        errs() << "Failed to add module: " << toString(std::move(Err)) << "\n";
        return false;
    }
    
    errs() << "JIT initialized successfully\n";
    return true;
}

int JITCompiler::executeMain() {
    if (!jit) {
        errs() << "JIT not initialized\n";
        return -1;
    }
    
    auto MainSym = jit->lookup("main");
    if (!MainSym) {
        errs() << "Main function not found: " << toString(MainSym.takeError()) << "\n";
        return -1;
    }
    
    auto MainAddr = MainSym->toPtr<int(*)()>();
    
    errs() << "\n=== Executing via JIT ===\n";
    
    int result = MainAddr();
    
    errs() << "=== JIT Execution Complete ===\n";
    errs() << "Return value: " << result << "\n\n";
    
    return result;
}