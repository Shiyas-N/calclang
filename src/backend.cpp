#include "backend.h"
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/IR/LegacyPassManager.h>
#include <system_error>
#include <cstdlib>
#include <optional>

using namespace llvm;

CodeGenBackend::CodeGenBackend(Module *M) 
    : module(M), targetMachine(nullptr) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmParser();
    InitializeNativeTargetAsmPrinter();
}

CodeGenBackend::~CodeGenBackend() {
    if (targetMachine) {
        delete targetMachine;
    }
}

bool CodeGenBackend::initializeTarget(const std::string& triple) {
    std::string targetTripleStr;
    
    if (triple.empty()) {
        targetTripleStr = sys::getDefaultTargetTriple();
    } else {
        targetTripleStr = triple;
    }
    
    Triple targetTriple(targetTripleStr);
    module->setTargetTriple(targetTriple);
    
    std::string error;
    const Target *target = TargetRegistry::lookupTarget(targetTripleStr, error);
    
    if (!target) {
        errs() << "Error: " << error << "\n";
        return false;
    }
    
    TargetOptions opt;
    
    targetMachine = target->createTargetMachine(
        targetTripleStr,
        "generic", 
        "",        
        opt,
        std::optional<Reloc::Model>(Reloc::PIC_) 
    );
    
    if (!targetMachine) {
        errs() << "Failed to create target machine\n";
        return false;
    }
    
    module->setDataLayout(targetMachine->createDataLayout());
    
    return true;
}

bool CodeGenBackend::emitFile(const std::string& filename, CodeGenFileType fileType) {
    if (!targetMachine) {
        errs() << "Target machine not initialized\n";
        return false;
    }
    
    std::error_code EC;
    raw_fd_ostream dest(filename, EC, sys::fs::OF_None);
    
    if (EC) {
        errs() << "Could not open file: " << EC.message() << "\n";
        return false;
    }
    
    legacy::PassManager pass;
    
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        errs() << "TargetMachine can't emit a file of this type\n";
        return false;
    }
    
    pass.run(*module);
    dest.flush();
    
    return true;
}

bool CodeGenBackend::emitObjectFile(const std::string& filename) {
    errs() << "Generating object file: " << filename << "\n";
    return emitFile(filename, CodeGenFileType::ObjectFile);
}

bool CodeGenBackend::emitAssembly(const std::string& filename) {
    errs() << "Generating assembly file: " << filename << "\n";
    return emitFile(filename, CodeGenFileType::AssemblyFile);
}

bool CodeGenBackend::linkExecutable(const std::string& objFile, const std::string& exeFile) {
    errs() << "Linking executable: " << exeFile << "\n";
    
#ifdef _WIN32
    std::string cmd = "clang " + objFile + " -o " + exeFile;
#else
    std::string cmd = "clang " + objFile + " -o " + exeFile;
#endif
    
    int result = std::system(cmd.c_str());
    
    if (result != 0) {
        errs() << "Linking failed with code: " << result << "\n";
        errs() << "Command: " << cmd << "\n";
        errs() << "Note: Make sure 'clang' is in your PATH\n";
        return false;
    }
    
    return true;
}

std::string CodeGenBackend::getTargetTriple() const {
    if (targetMachine) {
        return targetMachine->getTargetTriple().str();
    }
    return "";
}

void CodeGenBackend::printTargetInfo() {
    errs() << "\n=== Target Information ===\n";
    errs() << "Target Triple: " << getTargetTriple() << "\n";
    
    if (targetMachine) {
        errs() << "CPU: " << targetMachine->getTargetCPU().str() << "\n";
        errs() << "Features: " << targetMachine->getTargetFeatureString().str() << "\n";
    }
    
    errs() << "==========================\n\n";
}