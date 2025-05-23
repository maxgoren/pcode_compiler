#ifndef token_hpp
#define token_hpp
#include <iostream>
using namespace std;

enum Symbol {
    TK_ID, TK_NUM, TK_STR, TK_LP, TK_RP, TK_BEGIN, TK_END, TK_LB, TK_RB,
    TK_ADD, TK_MUL, TK_SUB, TK_DIV, TK_MOD, TK_POW, TK_SQRT,
    TK_LT, TK_GT, TK_LTE, TK_GTE, TK_EQU, TK_NEQ, TK_NOT, TK_AND, TK_OR,
    TK_PERIOD, TK_COMA, TK_SEMI, TK_COLON, TK_MATCH, TK_POST_INC, TK_POST_DEC,
    TK_ASSIGN, TK_QUOTE, TK_PROGRAM, TK_FUNC, TK_PRODUCES, TK_STRUCT, TK_NEW, TK_FREE,
    TK_LET, TK_VAR, TK_REF, TK_DO, TK_THEN, TK_PRINT, TK_WHILE, TK_RETURN, TK_IF, TK_ELSE,
    TK_EOI, TK_ERR,

    NT_PROGRAM, NT_STMTLIST, NT_STMT, NT_SIMPEXPR, NT_EXPR, NT_TERM, NT_FACTOR
};

string symbolStr[] = {
    "TK_ID", "TK_NUM", "TK_STR", "TK_LP", "TK_RP", "TK_BEGIN", "TK_END", "TK_LB", "TK_RB",
    "TK_ADD", "TK_MUL", "TK_SUB", "TK_DIV", "TK_MOD", "TK_POW", "TK_SQRT",
    "TK_LT", "TK_GT", "TK_LTE", "TK_GTE", "TK_EQU", "TK_NEQ", "TK_NOT", "TK_AND", "TK_OR",
    "TK_PERIOD", "TK_COMA", "TK_SEMI", "TK_COLON", "TK_MATCH", "TK_POST_INC", "TK_POST_DEC",
    "TK_ASSIGN", "TK_QUOTE", "TK_PROGRAM", "TK_AMPER", "TK_PRODUCES", "TK_STRUCT", "TK_NEW", "TK_FREE",
    "TK_LET", "TK_VAR", "TK_REF", "TK_DO", "TK_THEN", "TK_PRINT", "TK_WHILE", "TK_RETURN", "TK_IF", "TK_ELSE",
    "TK_EOI", "TK_ERR"
};

struct Token {
    Symbol symbol;
    string strval;
    Token(Symbol s = TK_EOI, string st = " ") : symbol(s), strval(st) { }
};

void printToken(Token tk) {
    cout<<"["<<symbolStr[tk.symbol]<<", "<<tk.strval<<"]"<<endl;
}


#endif