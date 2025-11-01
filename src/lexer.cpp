#include "lexer.h"
#include <cctype>
#include <iostream>

using namespace std;

Lexer::Lexer(const string& src): source(src),position(0),line(1),column(1){}


char Lexer::peek(){
    if(position>=source.length()) return '\0';
    return source[position];
}

char Lexer::advance(){
    if(position >=source.length()) return '\0';
    char c=source[position++];
    if(c=='\n'){
        line++;
        column=1;
    }else{
        column++;
    }
    return c;
}

void Lexer::skipWhitespace(){
    while(isspace(peek())){
        advance();
    }
}

void Lexer::skipComment() {
    if (peek() == '/' && position + 1 < source.length() && source[position + 1] == '/') {
        while (peek() != '\n' && peek() != '\0') {
            advance();
        }
    }
}

Token Lexer::readNumber() {
    int startLine = line;
    int startCol = column;
    string num;
    
    while (isdigit(peek())) {
        num += advance();
    }
    
    return Token(TOKEN_NUMBER, num, startLine, startCol);
}

Token Lexer::readIdentifier() {
    int startLine = line;
    int startCol = column;
    string id;
    
    while (isalnum(peek()) || peek() == '_') {
        id += advance();
    }
    
    if (id == "var") return Token(TOKEN_VAR, id, startLine, startCol);
    if (id == "func") return Token(TOKEN_FUNC, id, startLine, startCol);
    if (id == "return") return Token(TOKEN_RETURN, id, startLine, startCol);
    if (id == "print") return Token(TOKEN_PRINT, id, startLine, startCol);
    
    return Token(TOKEN_IDENTIFIER, id, startLine, startCol);
}

Token Lexer::getNextToken() {
    skipWhitespace();
    skipComment();
    skipWhitespace();
    
    int startLine = line;
    int startCol = column;
    
    if (peek() == '\0') {
        return Token(TOKEN_EOF, "", startLine, startCol);
    }
    
    if (isdigit(peek())) {
        return readNumber();
    }
    
    if (isalpha(peek()) || peek() == '_') {
        return readIdentifier();
    }
    
    char c = advance();
    switch (c) {
        case '+': return Token(TOKEN_PLUS, "+", startLine, startCol);
        case '-': return Token(TOKEN_MINUS, "-", startLine, startCol);
        case '*': return Token(TOKEN_MULTIPLY, "*", startLine, startCol);
        case '/': return Token(TOKEN_DIVIDE, "/", startLine, startCol);
        case '=': return Token(TOKEN_EQUALS, "=", startLine, startCol);
        case '(': return Token(TOKEN_LPAREN, "(", startLine, startCol);
        case ')': return Token(TOKEN_RPAREN, ")", startLine, startCol);
        case '{': return Token(TOKEN_LBRACE, "{", startLine, startCol);
        case '}': return Token(TOKEN_RBRACE, "}", startLine, startCol);
        case ';': return Token(TOKEN_SEMICOLON, ";", startLine, startCol);
        case ',': return Token(TOKEN_COMMA, ",", startLine, startCol);
        default:
            return Token(TOKEN_UNKNOWN, std::string(1, c), startLine, startCol);
    }
}

vector<Token> Lexer::tokenize() {
    vector<Token> tokens;
    Token token = getNextToken();
    
    while (token.type != TOKEN_EOF) {
        tokens.push_back(token);
        token = getNextToken();
    }
    
    tokens.push_back(token);
    return tokens;
}