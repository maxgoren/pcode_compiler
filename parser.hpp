#ifndef parser_hpp
#define parser_hpp
#include <iostream>
#include <vector>
#include "syntaxtree.hpp"
#include "token.hpp"
#include "tokenstream.hpp"
using namespace std;

bool isRelOp(Symbol sym) {
    switch (sym) {
        case TK_LT:
        case TK_LTE:
        case TK_GT:
        case TK_GTE:
        case TK_EQU:
        case TK_NEQ:
            return true;
        default:
            break;
    }
    return false;
}

class Parser {
    private:
        TokenStream ts;
        Token lookahead() {
            return ts.get();
        }
        void advance() {
            ts.advance();
        }
        bool expect(Symbol sym) {
            return sym == ts.get().symbol;
        }
        bool match(Symbol sym) {
            if (sym == ts.get().symbol) {
                cout<<"Match: "<<symbolStr[sym]<<endl;
                advance();
                return true;
            }
            return false;
        }
        
    public:
        Parser() {

        }
        ASTNode* parse(TokenStream tokenStream) {
            ts = tokenStream;
            ASTNode* node = program();
            return node;
        }
        ASTNode* program() {
            ASTNode* node = statementList();
            return node;
        }
        ASTNode* statementList() {
            ASTNode* node = statement();
            ASTNode* m = node;
            while (lookahead().symbol != TK_RC && lookahead().symbol != TK_EOI) {
                if (expect(TK_SEMI))
                    match(TK_SEMI);
                ASTNode* t = statement();
                if (m == nullptr) {
                    node = m = t;
                } else {
                    m->next = t;
                    m = t;
                }
            }
            return node;
        }
        ASTNode* statement() {
            ASTNode* node = nullptr;;
            switch (lookahead().symbol) {
                case TK_LET: {
                    node = makeStmtNode(LET_STMT, lookahead());
                    match(TK_LET);
                    node->data = lookahead();
                    match(TK_ID);
                    if (expect(TK_ASSIGN)) {
                        match(TK_ASSIGN);
                        node->child[0] = simpleExpr();
                    }
                } break;
                case TK_PRINT: {
                    node = makeStmtNode(PRINT_STMT, lookahead());
                    match(TK_PRINT);
                    node->child[0] = simpleExpr();
                } break;
                case TK_WHILE: {
                    node = makeStmtNode(WHILE_STMT, lookahead());
                    match(TK_WHILE);
                    match(TK_LP);
                    node->child[0] = simpleExpr();
                    match(TK_RP);
                    match(TK_LC);
                    node->child[1] = statementList();
                    match(TK_RC);
                } break;
                case TK_IF: {
                    node = makeStmtNode(IF_STMT, lookahead());
                    match(TK_IF);
                    match(TK_LP);
                    node->child[0] = simpleExpr();
                    match(TK_RP);
                    match(TK_LC);
                    node->child[1] = statementList();
                    match(TK_RC);
                    if (expect(TK_ELSE)) {
                        match(TK_ELSE);
                        match(TK_LC);
                        node->child[2] = statementList();
                        match(TK_RC);
                    }
                    return node;
                }
                case TK_RETURN: {
                    node = makeStmtNode(RETURN_STMT, lookahead());
                    match(TK_RETURN);
                    node->child[0] = simpleExpr();
                    return node;
                } break;
                case TK_AMPER:
                case TK_ID: 
                case TK_LP:
                case TK_NUM: {
                    node = makeStmtNode(EXPR_STMT, lookahead());
                    ASTNode* t = simpleExpr();
                    if (t->nk == STMT_NODE && t->type.stmt == ASSIGN_STMT) {
                        delete node;
                        return t;
                    }
                    node->child[0] = t;
                } break;
                default: break;
            }
            return node;
        }
        ASTNode* simpleExpr() {
            ASTNode* node = expression();
            while (isRelOp(lookahead().symbol)) {
                ASTNode* t = makeExprNode(RELOP_EXPR, lookahead());
                match(lookahead().symbol);
                t->child[0] = node;
                t->child[1] = expression();
                node = t;
            }
            return node;
        }
        ASTNode* expression() {
            ASTNode* node = term();
            while (expect(TK_ADD) || expect(TK_SUB)) {
                ASTNode* t = makeExprNode(BINOP_EXPR, lookahead());
                match(lookahead().symbol);
                t->child[0] = node;
                t->child[1] = term();
                node = t;
            }
            return node;
        }
        ASTNode* term() {
            ASTNode* node = prim();
            while (expect(TK_MUL) || expect(TK_DIV)) {
                ASTNode* t = makeExprNode(BINOP_EXPR, lookahead());
                match(lookahead().symbol);
                t->child[0] = node;
                t->child[1] = prim();
                node = t;
            }
            return node;
        }
        ASTNode* factor() {
            /*ASTNode* m = factor();
            while (expect(TK_SUB)) {
                ASTNode* t = makeExprNode(UNOP_EXPR, lookahead());
                t->child[0] = m;
                m = t;
            }*/
            return prim();
        }
        ASTNode* prim() {
            ASTNode* node = nullptr;
            if (expect(TK_NUM)) {
                node = makeExprNode(CONST_EXPR, lookahead());
                match(TK_NUM);
                return node;
            }
            if (expect(TK_SUB)) {
                node = makeExprNode(UNOP_EXPR, lookahead());
                match(TK_SUB);
                node->child[0] = simpleExpr();
                return node;
            }
            if (expect(TK_LP)) {
                match(TK_LP);
                node = simpleExpr();
                match(TK_RP);
                return node;
            }
            if (expect(TK_ID)) {
                node = makeExprNode(ID_EXPR, lookahead());
                match(TK_ID);
                if (expect(TK_ASSIGN)) {
                    node->nk = STMT_NODE;
                    node->type.stmt = ASSIGN_STMT;
                    match(TK_ASSIGN);
                    node->child[0] = simpleExpr();
                }
                if (expect(TK_LP)) {
                    ASTNode* t = makeExprNode(FUNC_EXPR, lookahead());
                    match(TK_LP);
                    t->data = node->data;
                    node = t;
                    if (!expect(TK_RP)) 
                        node->child[1] = argsList();
                    match(TK_RP);
                }
                return node;
            }
            if (expect(TK_STR)) {
                node = makeExprNode(STR_EXPR, lookahead());
                match(TK_STR);
                return node;
            }
            if (expect(TK_AMPER)) {
                node = makeExprNode(LAMBDA_EXPR, lookahead());
                match(TK_AMPER);
                if (expect(TK_ID)) {
                    node->data = lookahead();
                    match(TK_ID);
                }
                match(TK_LP);
                if (!expect(TK_RP)) {
                    node->child[0] = paramList();
                } else 
                match(TK_RP);
                if (expect(TK_LC)) {
                    match(TK_LC);
                    node->child[1] = statementList();
                    match(TK_RC);
                    return node;
                } else if (expect(TK_PRODUCES)) {
                    match(TK_PRODUCES);
                    node->child[1] = statement();
                }
                return node;
            }
            return node;
        }
        ASTNode* argsList() {
            ASTNode* m = simpleExpr();
            ASTNode* c = m;
            while (expect(TK_COMA)) {
                match(TK_COMA);
                c->next = simpleExpr();
                c = c->next;
            }
            return m;
        }
        ASTNode* paramList() {
                match(TK_LET);
                ASTNode* m = makeStmtNode(LET_STMT, lookahead());
                ASTNode* c = m;
                match(TK_ID);

            while (!expect(TK_RP)) {
                match(TK_COMA);
                match(TK_LET);
                c->next = makeStmtNode(LET_STMT, lookahead());
                c = c->next;
                match(TK_ID);
            }
            return m;
        }
};

#endif