#include "codegen.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <iostream>
#include <fstream>

using namespace std;

CodeGenerator::CodeGenerator(const string& moduleName) {
    context = make_unique<llvm::LLVMContext>();
    module = make_unique<llvm::Module>(moduleName, *context);
    builder = make_unique<llvm::IRBuilder<>>(*context);
    
    createPrintFunction();
}

void CodeGenerator::createPrintFunction() {
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context),
        llvm::PointerType::getUnqual(*context),
        true
    );
    
    llvm::Function* printfFunc = llvm::Function::Create(
        printfType,
        llvm::Function::ExternalLinkage,
        "printf",
        module.get()
    );
    
    functions["printf"] = printfFunc;
}

llvm::Value* CodeGenerator::codegen(NumberExprAST* expr) {
    return llvm::ConstantInt::get(*context, llvm::APInt(32, expr->value, true));
}

llvm::Value* CodeGenerator::codegen(VariableExprAST* expr) {
    llvm::Value* val = namedValues[expr->name];
    if (!val) {
        throw runtime_error("Unknown variable name: " + expr->name);
    }
    return builder->CreateLoad(llvm::Type::getInt32Ty(*context), val, expr->name);
}

llvm::Value* CodeGenerator::codegen(BinaryExprAST* expr) {
    llvm::Value* L = codegen(expr->left.get());
    llvm::Value* R = codegen(expr->right.get());
    
    if (!L || !R) return nullptr;
    
    switch (expr->op) {
        case '+':
            return builder->CreateAdd(L, R, "addtmp");
        case '-':
            return builder->CreateSub(L, R, "subtmp");
        case '*':
            return builder->CreateMul(L, R, "multmp");
        case '/':
            return builder->CreateSDiv(L, R, "divtmp");
        default:
            throw runtime_error("Invalid binary operator");
    }
}

llvm::Value* CodeGenerator::codegen(CallExprAST* expr) {
    llvm::Function* calleeF = functions[expr->callee];
    if (!calleeF) {
        throw runtime_error("Unknown function: " + expr->callee);
    }
    
    if (calleeF->arg_size() != expr->args.size()) {
        throw runtime_error("Incorrect number of arguments");
    }
    
    vector<llvm::Value*> argsV;
    for (auto& arg : expr->args) {
        argsV.push_back(codegen(arg.get()));
        if (!argsV.back()) return nullptr;
    }
    
    return builder->CreateCall(calleeF, argsV, "calltmp");
}

llvm::Value* CodeGenerator::codegen(ExprAST* expr) {
    if (auto* num = dynamic_cast<NumberExprAST*>(expr))
        return codegen(num);
    if (auto* var = dynamic_cast<VariableExprAST*>(expr))
        return codegen(var);
    if (auto* bin = dynamic_cast<BinaryExprAST*>(expr))
        return codegen(bin);
    if (auto* call = dynamic_cast<CallExprAST*>(expr))
        return codegen(call);
    
    return nullptr;
}

void CodeGenerator::codegen(VarDeclStmtAST* stmt) {
    llvm::Value* initVal = codegen(stmt->initializer.get());
    if (!initVal) return;
    
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    llvm::IRBuilder<> tmpBuilder(&func->getEntryBlock(), func->getEntryBlock().begin());
    
    llvm::AllocaInst* alloca = tmpBuilder.CreateAlloca(
        llvm::Type::getInt32Ty(*context), 
        nullptr, 
        stmt->name
    );
    
    builder->CreateStore(initVal, alloca);
    
    namedValues[stmt->name] = alloca;
}

void CodeGenerator::codegen(PrintStmtAST* stmt) {
    llvm::Value* val = codegen(stmt->expr.get());
    if (!val) return;
    
    llvm::Value* formatStr = builder->CreateGlobalStringPtr("%d\n");
    
    vector<llvm::Value*> printfArgs;
    printfArgs.push_back(formatStr);
    printfArgs.push_back(val);
    
    builder->CreateCall(functions["printf"], printfArgs);
}

void CodeGenerator::codegen(ReturnStmtAST* stmt) {
    llvm::Value* retVal = codegen(stmt->expr.get());
    if (retVal) {
        builder->CreateRet(retVal);
    }
}

void CodeGenerator::codegen(StmtAST* stmt) {
    if (auto* varDecl = dynamic_cast<VarDeclStmtAST*>(stmt))
        codegen(varDecl);
    else if (auto* print = dynamic_cast<PrintStmtAST*>(stmt))
        codegen(print);
    else if (auto* ret = dynamic_cast<ReturnStmtAST*>(stmt))
        codegen(ret);
}

llvm::Function* CodeGenerator::codegen(FunctionAST* func) {
    vector<llvm::Type*> paramTypes(func->params.size(), llvm::Type::getInt32Ty(*context));
    
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context),
        paramTypes,
        false
    );
    
    llvm::Function* function = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        func->name,
        module.get()
    );
    
    size_t idx = 0;
    for (auto& arg : function->args()) {
        arg.setName(func->params[idx++]);
    }
    
    llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(entryBB);
    
    namedValues.clear();
    
    for (auto& arg : function->args()) {
        llvm::AllocaInst* alloca = builder->CreateAlloca(
            llvm::Type::getInt32Ty(*context),
            nullptr,
            arg.getName()
        );
        builder->CreateStore(&arg, alloca);
        namedValues[string(arg.getName())] = alloca;
    }
    
    for (auto& stmt : func->body) {
        codegen(stmt.get());
    }
    
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateRet(llvm::ConstantInt::get(*context, llvm::APInt(32, 0)));
    }
    
    string errStr;
    llvm::raw_string_ostream errStream(errStr);
    if (llvm::verifyFunction(*function, &errStream)) {
        function->print(llvm::errs());
        throw runtime_error("Function verification failed: " + errStr);
    }
    
    functions[func->name] = function;
    return function;
}

void CodeGenerator::generate(ProgramAST* program) {
    llvm::FunctionType* mainType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context),
        false
    );
    
    llvm::Function* mainFunc = llvm::Function::Create(
        mainType,
        llvm::Function::ExternalLinkage,
        "main",
        module.get()
    );
    
    llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(*context, "entry", mainFunc);
    builder->SetInsertPoint(entryBB);
    
    for (auto& func : program->functions) {
        codegen(func.get());
    }
    
    builder->SetInsertPoint(entryBB);
    
    for (auto& stmt : program->statements) {
        codegen(stmt.get());
    }
    
    builder->CreateRet(llvm::ConstantInt::get(*context, llvm::APInt(32, 0)));
    
    string errStr;
    llvm::raw_string_ostream errStream(errStr);
    if (llvm::verifyModule(*module, &errStream)) {
        throw runtime_error("Module verification failed: " + errStr);
    }
}

void CodeGenerator::dump() {
    module->print(llvm::outs(), nullptr);
}

void CodeGenerator::writeToFile(const string& filename) {
    error_code EC;
    llvm::raw_fd_ostream file(filename, EC);
    
    if (EC) {
        throw runtime_error("Could not open file: " + filename);
    }
    
    module->print(file, nullptr);
}