#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <memory>
#include <vector>

using namespace std;

class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class ExprAST : public ASTNode {
public:
    virtual ~ExprAST() = default;
};

class NumberExprAST : public ExprAST {
public:
    int value;
    NumberExprAST(int val) : value(val) {}
};

class VariableExprAST : public ExprAST {
public:
    string name;
    VariableExprAST(const string& n) : name(n) {}
};

class BinaryExprAST : public ExprAST {
public:
    char op; 
    unique_ptr<ExprAST> left, right;
    
    BinaryExprAST(char operation, unique_ptr<ExprAST> l, unique_ptr<ExprAST> r)
        : op(operation), left(move(l)), right(move(r)) {}
};

class CallExprAST : public ExprAST {
public:
    string callee;
    vector<unique_ptr<ExprAST>> args;
    
    CallExprAST(const string& c, vector<unique_ptr<ExprAST>> a)
        : callee(c), args(move(a)) {}
};

class StmtAST : public ASTNode {
public:
    virtual ~StmtAST() = default;
};

class VarDeclStmtAST : public StmtAST {
public:
    string name;
    unique_ptr<ExprAST> initializer;
    
    VarDeclStmtAST(const string& n, unique_ptr<ExprAST> init)
        : name(n), initializer(move(init)) {}
};

class PrintStmtAST : public StmtAST {
public:
    unique_ptr<ExprAST> expr;
    
    PrintStmtAST(unique_ptr<ExprAST> e) : expr(move(e)) {}
};

class ReturnStmtAST : public StmtAST {
public:
    unique_ptr<ExprAST> expr;
    
    ReturnStmtAST(unique_ptr<ExprAST> e) : expr(move(e)) {}
};

class FunctionAST : public ASTNode {
public:
    string name;
    vector<string> params;
    vector<unique_ptr<StmtAST>> body;
    
    FunctionAST(const string& n, vector<string> p, vector<unique_ptr<StmtAST>> b)
        : name(n), params(move(p)), body(move(b)) {}
};

class ProgramAST : public ASTNode {
public:
    vector<unique_ptr<StmtAST>> statements;
    vector<unique_ptr<FunctionAST>> functions;
};

class Parser {
private:
    vector<Token> tokens;
    size_t current;
    
    Token peek();
    Token advance();
    bool match(TokenType type);
    bool check(TokenType type);
    Token consume(TokenType type, const string& message);
    
    unique_ptr<ExprAST> parseExpression();
    unique_ptr<ExprAST> parseAdditive();
    unique_ptr<ExprAST> parseMultiplicative();
    unique_ptr<ExprAST> parsePrimary();
    unique_ptr<StmtAST> parseStatement();
    unique_ptr<VarDeclStmtAST> parseVarDecl();
    unique_ptr<PrintStmtAST> parsePrint();
    unique_ptr<ReturnStmtAST> parseReturn();
    unique_ptr<FunctionAST> parseFunction();
    
public:
    Parser(const vector<Token>& toks);
    unique_ptr<ProgramAST> parse();
};

#endif