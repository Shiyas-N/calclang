#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <map>
#include <string>

using namespace std;

class CodeGenerator {
private:
    unique_ptr<llvm::LLVMContext> context;
    unique_ptr<llvm::Module> module;
    unique_ptr<llvm::IRBuilder<>> builder;
    
    map<string, llvm::Value*> namedValues;
    
    map<string, llvm::Function*> functions;
    
    llvm::Value* codegen(ExprAST* expr);
    llvm::Value* codegen(NumberExprAST* expr);
    llvm::Value* codegen(VariableExprAST* expr);
    llvm::Value* codegen(BinaryExprAST* expr);
    llvm::Value* codegen(CallExprAST* expr);
    
    void codegen(StmtAST* stmt);
    void codegen(VarDeclStmtAST* stmt);
    void codegen(PrintStmtAST* stmt);
    void codegen(ReturnStmtAST* stmt);
    
    llvm::Function* codegen(FunctionAST* func);
    
    void createPrintFunction();
    
public:
    CodeGenerator(const string& moduleName);
    
    void generate(ProgramAST* program);
    void dump(); 
    void writeToFile(const string& filename);
    
    llvm::Module* getModule() { return module.get(); }
};

#endif