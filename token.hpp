#ifndef token_hpp
#define token_hpp
#include <iostream>
using namespace std;

enum Symbol {
    TK_ID, TK_NUM, TK_STR, TK_LP, TK_RP, TK_LC, TK_RC, TK_LB, TK_RB,
    TK_ADD, TK_MUL, TK_SUB, TK_DIV, TK_MOD, TK_POW, TK_SQRT,
    TK_LT, TK_GT, TK_LTE, TK_GTE, TK_EQU, TK_NEQ,
    TK_PERIOD, TK_COMA, TK_SEMI, TK_COLON,
    TK_ASSIGN, TK_QUOTE, TK_AMPER, TK_PRODUCES, TK_STRUCT, TK_NEW, TK_FREE,
    TK_LET, TK_VAR, TK_DO, TK_THEN, TK_PRINT, TK_WHILE, TK_RETURN, TK_IF, TK_ELSE,
    TK_EOI, TK_ERR
};

string symbolStr[] = {
    "TK_ID", "TK_NUM", "TK_STR", "TK_LP", "TK_RP", "TK_LC", "TK_RC", "TK_LB", "TK_RB",
    "TK_ADD", "TK_MUL", "TK_SUB", "TK_DIV", "TK_MOD", "TK_POW", "TK_SQRT",
    "TK_LT", "TK_GT", "TK_LTE", "TK_GTE", "TK_EQU", "TK_NEQ",
    "TK_PERIOD", "TK_COMA", "TK_SEMI", "TK_COLON",
    "TK_ASSIGN", "TK_QUOTE", "TK_AMPER", "TK_PRODUCES", "TK_STRUCT", "TK_NEW", "TK_FREE",
    "TK_LET", "TK_VAR", "TK_DO", "TK_THEN", "TK_PRINT", "TK_WHILE", "TK_RETURN", "TK_IF", "TK_ELSE",
    "TK_EOI", "TK_ERR"
};

struct Token {
    Symbol symbol;
    string strval;
    Token(Symbol s = TK_EOI, string st = " ") : symbol(s), strval(st) { }
};

void printToken(Token tk) {
    cout<<"<["<<symbolStr[tk.symbol]<<", "<<tk.strval<<"]>"<<endl;
}


#endif