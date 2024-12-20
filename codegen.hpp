#ifndef codegen_hpp
#define codegen_hpp
#include <iostream>
#include <vector>
#include "syntaxtree.hpp"
#include "vminst.hpp"
#include "scoping_st.hpp"
using namespace std;





class PCodeGenerator {
    private:
        bool should_trace;
        ScopingSymbolTable st;
        string makeLabel() {
            static int labelnum = 0;
            labelnum++;
            return "L" + to_string(labelnum);
        }
        vector<Instruction> codepage;
        bool genparam;
        int cPos;
        void emit(Inst op, Value operand, Value nestLevel) {
            codepage[cPos] = Instruction(op, operand, nestLevel);
            cPos++;
        }
        void emit(Inst op, Value operand) {
            codepage[cPos] = Instruction(op, operand);
            cPos++;
        }
        void emit(Inst inst) {
            codepage[cPos] = Instruction(inst);
            cPos++;
        }
        void genIfStmt(ASTNode* node, bool isAddr) {
            string l1, l2;
            genCode(node->child[0], isAddr);
            l1 = makeLabel();
            emit(JPC, makeString(l1));
            genCode(node->child[1], isAddr);
            if (node->child[2] != nullptr) {
                l2 = makeLabel();
                emit(JMP, makeString(l2));
            }
            emit(LAB, makeString(l1));
            if (node->child[2] != nullptr) {
                genCode(node->child[2], isAddr);
                emit(LAB, makeString(l2));
            }
        }
        void genWhileStmt(ASTNode* node, bool isAddr) {
            string l1;
            string l2;
            l1 = makeLabel();
            emit(LAB, makeString(l1));
            genCode(node->child[0], isAddr);
            l2 = makeLabel();
            emit(JPC, makeString(l2));
            genCode(node->child[1], isAddr);
            emit(JMP, makeString(l1));
            emit(LAB, makeString(l2));
        }
        void genStmt(ASTNode* node, bool isAddr) {
            switch (node->type.stmt) {
                case EXPR_STMT: {
                    genCode(node->child[0], isAddr);
                } break;
                case PRINT_STMT: {
                    genCode(node->child[0], isAddr);
                    emit(PRINT);
                } break;
                case LET_STMT: {
                    LocalVar* lv = st.getVar(node->data.strval);
                    emit(LDA, makeInt(lv->loc), makeInt(lv->depth));
                    genCodeNS(node->child[0],false);
                    emit(STO);
                } break;
                case IF_STMT: {
                    genIfStmt(node, isAddr);
                } break;
                case WHILE_STMT: {
                    genWhileStmt(node, isAddr);
                } break;
                case RETURN_STMT: {
                    genCode(node->child[0], isAddr);
                }
                default: break;
            }
        }
        void genBinOp(ASTNode* node, bool isAddr) {
            genCode(node->child[0], isAddr);
            genCode(node->child[1], isAddr);
            switch (node->data.symbol) {
                case TK_ADD: emit(ADD); break;
                case TK_SUB: emit(SUB); break;
                case TK_MUL: emit(MUL); break;
                case TK_DIV: emit(DIV); break;
                default: break;
            }
        }
        void genRelOp(ASTNode* node, bool isAddr) {
            genCode(node->child[0], isAddr);
            genCode(node->child[1], isAddr);
            switch (node->data.symbol) {
                case TK_LT: emit(LT); break;
                case TK_LTE: emit(LTE); break;
                case TK_GT:  emit(GT); break;
                case TK_GTE: emit(GTE); break;
                case TK_EQU: emit(EQU); break;
                case TK_NEQ: emit(NEQ); break;
                default:
                    break;
            }
        }
        bool hasSubscript(ASTNode* node) {
            return (node->child[0] != nullptr && node->child[0]->nk == EXPR_NODE && node->child[0]->type.expr == SUBSCRIPT_EXPR);
        }
        bool hasField(ASTNode* node) {
            return (node->child[0] != nullptr && node->child[0]->nk == EXPR_NODE && node->child[0]->type.expr == FIELD_EXPR);
        }
        bool isField;
        void genExpr(ASTNode* node, bool isAddr) {
            switch (node->type.expr) {
                case ID_EXPR: {
                    LocalVar* lv = st.getVar(node->data.strval);
                    if (lv == nullptr) {
                        cout<<"Error: attempt to reference undelcared variable: "<<node->data.strval<<endl;
                        emit(HALT);
                        return;
                    }
                    if (isField) {
                        emit(LDF, makeInt(lv->loc));
                    } else if (isAddr || hasSubscript(node) || hasField(node)) {
                        emit(LDA, makeInt(lv->loc), makeInt(0));
                    } else {
                        emit(genparam ? LDP:LOD, makeInt(lv->loc), makeInt(0));
                    }
                    if (node->child[LEFTCHILD] != nullptr) {
                        if (hasField(node)) {
                            isField = true;
                            st.openStruct(node);
                            genExpr(node->child[LEFTCHILD], isAddr);
                            st.closeStruct();
                            isField = false;
                        } else {
                            genExpr(node->child[LEFTCHILD], isAddr);
                        }
                    }
                } break;
                case FIELD_EXPR: {
                    genCodeNS(node->child[LEFTCHILD], true);
                    emit(IXA, makeInt(1), makeInt(0));
                    if (!isAddr) emit(LDI, makeInt(0));
                } break;
                case SUBSCRIPT_EXPR: {
                    genCodeNS(node->child[LEFTCHILD], false);
                    emit(IXA, makeInt(1), makeInt(0));
                    if (!isAddr) emit(LDI, makeInt(0));
                } break;
                case CONST_EXPR: {
                    emit(LDC, makeReal(stod(node->data.strval)));
                } break;
                case BINOP_EXPR: {
                    genBinOp(node, isAddr);
                } break;
                case UNOP_EXPR: {
                    genCode(node->child[0], isAddr);
                    emit(NEG);
                } break;
                case RELOP_EXPR: {
                    genRelOp(node, isAddr);
                } break;
                case STR_EXPR: {
                    emit(LDC, makeString(node->data.strval));
                } break;
                case ASSIGN_EXPR: {
                    ASTNode* idPart = node->child[0];
                    LocalVar* lv = st.getVar(idPart->data.strval);
                    int offset = 0;
                    genExpr(node->child[LEFTCHILD], true);
                    genExpr(node->child[RIGHTCHILD], false);
                    emit(STO);
                } break;
                case LAMBDA_EXPR: {
                    string pre = makeLabel();
                    string after = makeLabel();
                    st.openScope(node->data.strval);
                    emit(JMP, makeString(after));
                    emit(ENT, makeString(node->data.strval));
                    genCode(node->child[1], isAddr);
                    emit(RET);
                    emit(LAB, makeString(after));
                    st.closeScope();
                } break;
                case FUNC_EXPR: {
                    emit(MST);
                    ASTNode* t = node->child[1];
                    genparam = true;
                    while (t != nullptr) {
                        genCodeNS(t, isAddr);
                        t = t->next;
                    }
                    genparam = false;
                    emit(CAL, makeString(node->data.strval));
                } break;
                default:
                    break;
            }
        }
        void genCodeNS(ASTNode* node, bool isAddr) {
            if (node != nullptr) {
                switch (node->nk) {
                    case EXPR_NODE: genExpr(node, isAddr); break;
                    case STMT_NODE: genStmt(node, isAddr); break;
                    default: break;
                }
            }
        }
        void genCodeParam(ASTNode* node) {
            LocalVar* lv = st.getVar(node->data.strval);
            emit(LDP, makeInt(lv->loc), makeInt(0));
        }
        void genCode(ASTNode* node, bool isAddr) {
            if (node != nullptr) {
                switch (node->nk) {
                    case EXPR_NODE: genExpr(node, isAddr); break;
                    case STMT_NODE: genStmt(node, isAddr); break;
                    default: break;
                }
                genCode(node->next, isAddr);
            }
        }
        void buildST(ASTNode* node) {
            if (node != nullptr) {
                switch (node->nk) {
                    case STMT_NODE: 
                        switch (node->type.stmt) {
                            case LET_STMT: {
                                if (st.getVar(node->data.strval) == nullptr) {
                                    if (hasSubscript(node)) {
                                        st.insertVar(node->data.strval, atoi(node->child[0]->data.strval.data()));
                                        if (should_trace)
                                            cout<<node->data.strval<<" added to symbol table as an array of size "<<atoi(node->child[0]->data.strval.data())<<endl;
                                    } else {
                                        st.insertVar(node->data.strval);
                                        if (should_trace)
                                            cout<<node->data.strval<<" added to symbol table"<<endl;
                                    }
                                }                                
                            } break;
                            case STRUCT_STMT: {
                                if (st.getVar(node->data.strval) == nullptr) {
                                    st.openStruct(node);
                                    buildST(node->child[0]);
                                    buildST(node->child[1]);
                                    st.closeStruct();
                                    buildST(node->next);
                                    if (should_trace)
                                        cout<<node->data.strval<<" added to symbol table"<<endl;
                                    return;
                                }
                            } break;
                        };
                        break;
                    case EXPR_NODE: 
                        switch (node->type.expr) {
                            case ID_EXPR: {
                                if (hasField(node)) {
                                    ASTNode* t = node->child[0];
                                    Scope* ts = st.getStruct(node->data.strval);
                                    if (ts != nullptr) {
                                        if (should_trace)
                                            cout<<"Getting field "<<t->child[0]->data.strval<<" ";
                                        LocalVar* field = st.getFieldFromStruct(ts, t->child[0]->data.strval);
                                        if (field != nullptr) {
                                            return;
                                        }
                                    }
                                }
                                if (st.getVar(node->data.strval) == nullptr) {
                                    cout<<"Error: undeclared ass variable trying to be used: "<<node->data.strval<<endl;
                                }     
                            } break;
                            case LAMBDA_EXPR: {
                                st.openScope(node->data.strval);
                                buildST(node->child[0]);
                                buildST(node->child[1]);
                                st.closeScope();
                                buildST(node->next);
                                return;
                            } break;
                        } break;
                    default: break;
                }
                for (int i = 0; i < 3; i++)
                    buildST(node->child[i]);
                buildST(node->next);
            }
        }
        void init() {
            codepage = vector<Instruction>(1000);
            cPos = 0;
            isField = false;
        }
    public:
        PCodeGenerator(bool trace = false) {
            init();
            genparam = false;
            isField = false;
            should_trace = trace;
        }
        void setContext(ScopingSymbolTable& symbolTable) {
            st = symbolTable;
        }
        void setTrace(bool trace) {
            should_trace = trace;
            st.setTrace(trace);
        }
        vector<Instruction> generate(ASTNode* node) {
            init();
            if (should_trace)
                cout<<"Building Symbol Table: "<<endl;
            buildST(node);
            if (should_trace )
                st.print();
            if (should_trace)
                cout<<"Generating P-Code..."<<endl;
            genCode(node, false);
            emit(HALT);
            codepage.resize(cPos);
            if (should_trace)
                cout<<"Done."<<endl;
            return codepage;
        }
};


#endif