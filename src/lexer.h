#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

using namespace std;

enum TokenType{
    TOKEN_EOF,
    TOKEN_VAR,
    TOKEN_FUNC,
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_EQUALS,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_UNKNOWN
};

struct Token{
    TokenType type;
    string value;
    int line;
    int column;

    Token(TokenType t,const string& v,int l,int c):type(t),value(v),line(l),column(c){}
};

class Lexer{
    private:
        string source;
        size_t position;
        int line;
        int column;

        char peek();
        char advance();
        void skipWhitespace();
        void skipComment();
        Token readNumber();
        Token readIdentifier();

    public:
        Lexer(const string& src);
        Token getNextToken();
        vector<Token> tokenize();
};



#endif