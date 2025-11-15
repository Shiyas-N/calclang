#ifndef BACKEND_H
#define BACKEND_H

#include <llvm/IR/Module.h>
#include <llvm/Target/TargetMachine.h>
#include <string>

using namespace std;

class CodeGenBackend {
public:
    CodeGenBackend(llvm::Module *M);
    ~CodeGenBackend();
    
    bool initializeTarget(const string& triple = "");
    
    bool emitObjectFile(const string& filename);
    
    bool emitAssembly(const string& filename);
    
    bool linkExecutable(const string& objFile, const string& exeFile);
    
    string getTargetTriple() const;
    
    void printTargetInfo();
    
private:
    llvm::Module *module;
    llvm::TargetMachine *targetMachine;
    
    bool emitFile(const string& filename, llvm::CodeGenFileType fileType);
};

#endif