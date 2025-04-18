#ifndef parser_hpp
#define parser_hpp
#include <iostream>
#include <vector>
#include "scoping_st.hpp"
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
                //cout<<"Match: "<<symbolStr[sym]<<endl;
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
    private:
        ASTNode* program() {
            ASTNode* program;
            if (expect(TK_PROGRAM)) {
                program = makeStmtNode(PROGRAM_STMT, lookahead());
                match(TK_PROGRAM);
                match(TK_ID);
                match(TK_BEGIN);
                program->child[0] = statementList();
                match(TK_END);
                match(TK_PERIOD);
                return program;
            }
            ASTNode* node = statementList();
            return node;
        }
        ASTNode* statementList() {
            ASTNode* node = statement();
            ASTNode* m = node;
            while (lookahead().symbol != TK_END && lookahead().symbol != TK_EOI) {
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
                    node = letStatement();
                } break;
                case TK_STRUCT: {
                    node = defineStruct();
                } break;
                case TK_PRINT: {
                    node = makeStmtNode(PRINT_STMT, lookahead());
                    match(TK_PRINT);
                    node->child[0] = simpleExpr();
                } break;
                case TK_WHILE: {
                    node = whileStatement();
                } break;
                case TK_IF: {
                    node = ifStatement();
                } break;
                case TK_RETURN: {
                    node = makeStmtNode(RETURN_STMT, lookahead());
                    match(TK_RETURN);
                    node->child[0] = simpleExpr();
                } break;
                case TK_FUNC: {
                    node = functionDefinition();
                } break;
                case TK_MATCH:
                case TK_ID: 
                case TK_LP:
                case TK_NUM: {
                    node = makeStmtNode(EXPR_STMT, lookahead());
                    ASTNode* t = simpleExpr();
                    node->child[0] = t;
                } break;
                default: break;
            }
            return node;
        }
        ASTNode* whileStatement() {
            ASTNode* node = makeStmtNode(WHILE_STMT, lookahead());
            match(TK_WHILE);
            match(TK_LP);
            node->child[0] = simpleExpr();
            match(TK_RP);
            if (expect(TK_DO)) match(TK_DO);
            node->child[1] = makeBlock();
            return node;
        }
        ASTNode* ifStatement() {
            ASTNode* node = makeStmtNode(IF_STMT, lookahead());
            match(TK_IF);
            match(TK_LP);
            node->child[0] = simpleExpr();
            match(TK_RP);
            if (expect(TK_THEN)) match(TK_THEN);
            node->child[1] = makeBlock();
            if (expect(TK_ELSE)) {
                match(TK_ELSE);
                node->child[2] = makeBlock();
            }
            return node;
        }
        ASTNode* defineStruct() {
            ASTNode* node = makeStmtNode(STRUCT_STMT, lookahead());
            match(TK_STRUCT);
            node->data.strval = lookahead().strval;
            match(TK_ID);
            node->child[0] = makeBlock();
            return node;
        }
        ASTNode* functionDefinition() {
            ASTNode* node = nullptr;
            if (expect(TK_FUNC)) {
                node = makeStmtNode(FUNC_DEF_STMT, lookahead());
                match(TK_FUNC);
                if (expect(TK_ID)) {
                    node->data = lookahead();
                    match(TK_ID);
                }
                match(TK_LP);
                if (!expect(TK_RP)) {
                    node->child[0] = paramList();
                    match(TK_RP);
                } else {
                    match(TK_RP);
                }
                if (expect(TK_BEGIN)) {
                    node->child[1] = makeBlock();
                } 
            }
            return node;
        }
        ASTNode* makeBlock() {
            ASTNode* node;
            match(TK_BEGIN);
            node = statementList();
            match(TK_END);
            return node;
        }
        ASTNode* letStatement() {
            ASTNode* node = makeStmtNode(LET_STMT, lookahead());
            match(TK_LET);
            node->data = lookahead();
            match(TK_ID);
            if (expect(TK_LB)) {
                match(TK_LB);
                node->child[0] = makeExprNode(SUBSCRIPT_EXPR, lookahead());
                node->child[0]->child[0] = simpleExpr();
                match(TK_RB);
            } else if (expect(TK_ASSIGN)) {
                match(TK_ASSIGN);
                node->child[0] = simpleExpr();
            }
            return node;
        }
        ASTNode* simpleExpr() {
            ASTNode* node = relExpr();
            if (expect(TK_ASSIGN)) {
                ASTNode* t = makeExprNode(ASSIGN_EXPR, lookahead());
                match(TK_ASSIGN);
                t->child[0] = node;
                node = t;
                node->child[1] = relExpr();
            }
            return node;
        }
        ASTNode* relExpr() {
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
            ASTNode* node = factor();
            while (expect(TK_MUL) || expect(TK_DIV)) {
                ASTNode* t = makeExprNode(BINOP_EXPR, lookahead());
                match(lookahead().symbol);
                t->child[0] = node;
                t->child[1] = factor();
                node = t;
            }
            return node;
        }
        ASTNode* factor() {
            ASTNode* node;
            if (expect(TK_SUB)) {
                node = makeExprNode(UNOP_EXPR, lookahead());
                match(TK_SUB);
                node->child[0] = factor();
                return node;
            }
            if (expect(TK_NOT)) {
                node = makeExprNode(UNOP_EXPR, lookahead());
                match(TK_NOT);
                node->child[0] = factor();
                return node;
            }
            node = primary();
            return node;
        }
        ASTNode* primary() {
            ASTNode* node = val();
            if (expect(TK_LB)) {
                while (expect(TK_LB)) {
                    ASTNode* t = makeExprNode(SUBSCRIPT_EXPR, lookahead());
                    match(TK_LB);
                    t->child[0] = simpleExpr();
                    match(TK_RB);
                    node->child[0] = t;
                }
            } else if (expect(TK_PERIOD)) {
                while (expect(TK_PERIOD)) {
                    ASTNode* t = makeExprNode(FIELD_EXPR, lookahead());
                    match(TK_PERIOD);
                    t->child[0] = makeExprNode(ID_EXPR, lookahead());
                    match(TK_ID);
                    node->child[0] = t;
                }
            } else if (expect(TK_POST_INC) || expect(TK_POST_DEC)) {
                ASTNode* t = makeExprNode(UNOP_EXPR, lookahead());
                match(lookahead().symbol);
                t->child[0] = node;
                node = t;
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
        ASTNode* val() {
            ASTNode* node = nullptr;
            if (expect(TK_NUM)) {
                node = makeExprNode(CONST_EXPR, lookahead());
                match(TK_NUM);
                return node;
            }
            if (expect(TK_ID)) {
                node = makeExprNode(ID_EXPR, lookahead());
                match(TK_ID);
                return node;
            }
            if (expect(TK_STR)) {
                node = makeExprNode(STR_EXPR, lookahead());
                match(TK_STR);
                return node;
            }
            if (expect(TK_LP)) {
                match(TK_LP);
                node = simpleExpr();
                match(TK_RP);
                return node;
            }
            if (expect(TK_NEW)) {
                node = makeExprNode(BLESS_EXPR, lookahead());
                match(TK_NEW);
                node->child[0] = simpleExpr();
                return node;
            }
            if (expect(TK_MATCH)) {
                node = makeExprNode(REG_EXPR, lookahead());
                match(TK_MATCH);
                match(TK_LP);
                node->child[0] = simpleExpr();
                match(TK_COMA);
                node->child[1] = simpleExpr();
                match(TK_RP);
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
            if (expect(TK_REF)) {
                match(TK_REF);
                m->type.stmt = REF_STMT;
                m->data = lookahead();
            }
            match(TK_ID);
            while (!expect(TK_RP)) {
                match(TK_COMA);
                match(TK_LET);
                c->next = makeStmtNode(LET_STMT, lookahead());
                c = c->next;
                if (expect(TK_REF)) {
                    match(TK_REF);
                    c->type.stmt = REF_STMT;
                    c->data = lookahead();
                }
                match(TK_ID);
            }
            return m;
        }
};

#endif