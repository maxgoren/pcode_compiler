#ifndef shunting_hpp
#define shunting_hpp
#include <stack>
#include "token.hpp"
#include "tokenstream.hpp"
#include "syntaxtree.hpp"
using namespace std;

bool isOp(Symbol c) {
    switch (c) {
        case TK_MUL: return true;
        case TK_DIV: return true;
        case TK_ADD: return true;
        case TK_SUB: return true;
        default:
            break;
    }
    return false;
}

bool leftAssociates(Symbol c) {
    switch (c) {
        case TK_MUL: return true;
        case TK_DIV: return true;
        case TK_ADD: return true;
        case TK_SUB: return true;
        default:
            break;
    }
    return false;
}

int precedence(Symbol c) {
    switch (c) {
        case TK_MUL: return 5;
        case TK_DIV: return 5;
        case TK_ADD: return 3;
        case TK_SUB: return 3;
        default:
            break;
    }
    return 0;
} 

class ShuntingYard {
    private:
        stack<Token> sf;
        void push(Token tk) {
            sf.push(tk);
        }
        Token pop() {
            Token t = sf.top();
            sf.pop();
            return t;
        }
    public:
        ShuntingYard() {

        }
        TokenStream in2post(TokenStream ts) {
            vector<Token> output;
            while (!ts.done()) {
                if (ts.get().symbol == TK_LP) {
                    sf.push(ts.get());
                } else if (isOp(ts.get().symbol)) {
                    if (precedence(ts.get().symbol) < precedence(sf.top().symbol) || (precedence(ts.get().symbol) == precedence(sf.top().symbol) && leftAssociates(sf.top().symbol))) {
                        output.push_back(pop());
                        push(ts.get());
                    } else {
                        push(ts.get());
                    }
                } else if (ts.get().symbol == TK_RP) {
                    while (!sf.empty()) {
                        if (sf.top().symbol != TK_LP) {
                            output.push_back(pop());
                        } else {
                            sf.pop();
                            break;
                        }
                    }
                } else {
                    output.push_back(pop());
                }
                ts.advance();
            }
            while (!sf.empty()) {
                if (sf.top().symbol != TK_LP) {
                    output.push_back(pop());
                } else {
                    sf.pop();
                }
            }
            return TokenStream(output);
        }
        ASTNode* post2ast(TokenStream ts) {
            stack<ASTNode*> sf;
            while (!ts.done()) {
                switch (ts.get().symbol) {
                    case TK_ID: {
                        sf.push(makeExprNode(ID_EXPR, ts.get()));
                    } break;
                    case TK_NUM: {
                        sf.push(makeExprNode(CONST_EXPR, ts.get()));
                    }
                } 
            }
        }
};

#endif