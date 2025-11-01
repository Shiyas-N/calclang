#include "parser.h"
#include <stdexcept>
#include <iostream>

using namespace std;

Parser::Parser(const vector<Token>& toks) : tokens(toks), current(0) {}

Token Parser::peek() {
    if (current >= tokens.size()) return tokens.back();
    return tokens[current];
}

Token Parser::advance() {
    if (current < tokens.size()) current++;
    return tokens[current - 1];
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) {
    if (current >= tokens.size()) return false;
    return peek().type == type;
}

Token Parser::consume(TokenType type, const string& message) {
    if (check(type)) return advance();
    throw runtime_error(message + " at line " + to_string(peek().line));
}

unique_ptr<ExprAST> Parser::parseExpression() {
    return parseAdditive();
}

unique_ptr<ExprAST> Parser::parseAdditive() {
    auto left = parseMultiplicative();
    
    while (match(TOKEN_PLUS) || match(TOKEN_MINUS)) {
        char op = tokens[current - 1].value[0];
        auto right = parseMultiplicative();
        left = make_unique<BinaryExprAST>(op, move(left), move(right));
    }
    
    return left;
}

unique_ptr<ExprAST> Parser::parseMultiplicative() {
    auto left = parsePrimary();
    
    while (match(TOKEN_MULTIPLY) || match(TOKEN_DIVIDE)) {
        char op = tokens[current - 1].value[0];
        auto right = parsePrimary();
        left = make_unique<BinaryExprAST>(op, move(left), move(right));
    }
    
    return left;
}

unique_ptr<ExprAST> Parser::parsePrimary() {
    if (match(TOKEN_NUMBER)) {
        int value = stoi(tokens[current - 1].value);
        return make_unique<NumberExprAST>(value);
    }
    
    if (match(TOKEN_IDENTIFIER)) {
        string name = tokens[current - 1].value;
        
        if (match(TOKEN_LPAREN)) {
            vector<unique_ptr<ExprAST>> args;
            
            if (!check(TOKEN_RPAREN)) {
                do {
                    args.push_back(parseExpression());
                } while (match(TOKEN_COMMA));
            }
            
            consume(TOKEN_RPAREN, "Expected ')' after arguments");
            return make_unique<CallExprAST>(name, move(args));
        }
        
        return make_unique<VariableExprAST>(name);
    }
    
    if (match(TOKEN_LPAREN)) {
        auto expr = parseExpression();
        consume(TOKEN_RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    throw runtime_error("Unexpected token in expression at line " + to_string(peek().line));
}

unique_ptr<VarDeclStmtAST> Parser::parseVarDecl() {
    consume(TOKEN_VAR, "Expected 'var'");
    Token nameToken = consume(TOKEN_IDENTIFIER, "Expected variable name");
    consume(TOKEN_EQUALS, "Expected '=' after variable name");
    auto init = parseExpression();
    consume(TOKEN_SEMICOLON, "Expected ';' after variable declaration");
    
    return make_unique<VarDeclStmtAST>(nameToken.value, move(init));
}

unique_ptr<PrintStmtAST> Parser::parsePrint() {
    consume(TOKEN_PRINT, "Expected 'print'");
    consume(TOKEN_LPAREN, "Expected '(' after 'print'");
    auto expr = parseExpression();
    consume(TOKEN_RPAREN, "Expected ')' after expression");
    consume(TOKEN_SEMICOLON, "Expected ';' after print statement");
    
    return make_unique<PrintStmtAST>(move(expr));
}

unique_ptr<ReturnStmtAST> Parser::parseReturn() {
    consume(TOKEN_RETURN, "Expected 'return'");
    auto expr = parseExpression();
    consume(TOKEN_SEMICOLON, "Expected ';' after return statement");
    
    return make_unique<ReturnStmtAST>(move(expr));
}

unique_ptr<StmtAST> Parser::parseStatement() {
    if (check(TOKEN_VAR)) return parseVarDecl();
    if (check(TOKEN_PRINT)) return parsePrint();
    if (check(TOKEN_RETURN)) return parseReturn();
    
    throw runtime_error("Unexpected statement at line " + to_string(peek().line));
}

unique_ptr<FunctionAST> Parser::parseFunction() {
    consume(TOKEN_FUNC, "Expected 'func'");
    Token nameToken = consume(TOKEN_IDENTIFIER, "Expected function name");
    
    consume(TOKEN_LPAREN, "Expected '(' after function name");
    vector<string> params;
    
    if (!check(TOKEN_RPAREN)) {
        do {
            Token param = consume(TOKEN_IDENTIFIER, "Expected parameter name");
            params.push_back(param.value);
        } while (match(TOKEN_COMMA));
    }
    
    consume(TOKEN_RPAREN, "Expected ')' after parameters");
    consume(TOKEN_LBRACE, "Expected '{' before function body");
    
    vector<unique_ptr<StmtAST>> body;
    while (!check(TOKEN_RBRACE) && !check(TOKEN_EOF)) {
        body.push_back(parseStatement());
    }
    
    consume(TOKEN_RBRACE, "Expected '}' after function body");
    
    return make_unique<FunctionAST>(nameToken.value, move(params), move(body));
}

unique_ptr<ProgramAST> Parser::parse() {
    auto program = make_unique<ProgramAST>();
    
    while (!check(TOKEN_EOF)) {
        if (check(TOKEN_FUNC)) {
            program->functions.push_back(parseFunction());
        } else {
            program->statements.push_back(parseStatement());
        }
    }
    
    return program;
}