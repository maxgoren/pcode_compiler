#ifndef syntaxtree_hpp
#define syntaxtree_hpp
#include "token.hpp"

enum NodeKind {
    EXPR_NODE, STMT_NODE
};

enum ExprType {
    CONST_EXPR, STR_EXPR, ID_EXPR, UNOP_EXPR, BINOP_EXPR, RELOP_EXPR, LAMBDA_EXPR, FUNC_EXPR, SUBSCRIPT_EXPR, FIELD_EXPR, ASSIGN_EXPR, BLESS_EXPR
};

enum StmtType {
    PROGRAM_STMT, PRINT_STMT, EXPR_STMT, LET_STMT, REF_STMT, WHILE_STMT, IF_STMT, RETURN_STMT, STRUCT_STMT
};

string nodeKindStr[] = {
    "EXPR_NODE", "STMT_NODE"
};

string exprTypeStr[] = {
    "CONST_EXPR", "STR_EXPR", "ID_EXPR", "UNOP_EXPR", "BINOP_EXPR", "RELOP_EXPR", "LAMBDA_EXPR", "FUNC_EXPR", "SUBSCRIPT_EXPR", "FIELD_EXPR", "ASSIGN_EXPR", "BLESS_EXPR"
};

string stmtTypeStr[] = {
    "PROGRAM_STMT", "PRINT_STMT", "EXPR_STMT", "LET_STMT", "REF_STMT", "WHILE_STMT", "IF_STMT", "RETURN_STMT", "STRUCT_STMT"
};

const int MAXCHILD = 3;
const int LEFTCHILD = 0;
const int RIGHTCHILD = 1;


enum ScopeType {
    LOCAL, GLOBAL
};

struct Attributes {
    ScopeType scoping;
    int nestLevel;
    int loc;
    Attributes() : scoping(GLOBAL), nestLevel(0), loc(0) { }
};

struct ASTNode {
    NodeKind nk;
    union {
        ExprType expr;
        StmtType stmt;
    } type;
    Attributes attributes;
    Token data;
    ASTNode* next; 
    ASTNode* child[MAXCHILD];
    ASTNode() {
        next = nullptr;
        for (int i = 0; i < MAXCHILD; i++)
            child[i] = nullptr;
    }
};

ASTNode* makeExprNode(ExprType et, Token t) {
    ASTNode* nn = new ASTNode();
    nn->nk = EXPR_NODE;
    nn->type.expr = et;
    nn->data = t;
    return nn;
}

ASTNode* makeStmtNode(StmtType st, Token t) {
    ASTNode* nn = new ASTNode();
    nn->nk = STMT_NODE;
    nn->type.stmt = st;
    nn->data = t;
    return nn;
}

int depth = 0;
void traverse(ASTNode* node) {
    depth++;
    if (node != nullptr) {
        for (int i = 0; i < depth; i++) cout<<" ";
        if (node->nk == EXPR_NODE) {
            cout<<"["<<exprTypeStr[node->type.expr]<<"] ";
        } else if (node->nk == STMT_NODE) {
            cout<<"["<<stmtTypeStr[node->type.stmt]<<"] ";
        } else {
            depth--;
            return;
        }
        printToken(node->data);
        for (int i = 0; i < MAXCHILD; i++)
            traverse(node->child[i]);
        traverse(node->next);
    }
    depth--;
}

void cleanup(ASTNode* node) {
    if (node != nullptr) {
        cleanup(node->next);
        for (int i = 0; i < MAXCHILD; i++)
            cleanup(node->child[0]);
        delete node;
    }
}

#endif