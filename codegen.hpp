#ifndef codegen_hpp
#define codegen_hpp
#include <iostream>
#include <vector>
#include "syntaxtree.hpp"
#include "vminst.hpp"
#include "symboltable.hpp"
#include "scoping_st.hpp"
using namespace std;





class PCodeGenerator {
    private:
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
        void genIfStmt(ASTNode* node) {
            string l1, l2;
            genCode(node->child[0]);
            l1 = makeLabel();
            emit(JPC, makeString(l1));
            genCode(node->child[1]);
            if (node->child[2] != nullptr) {
                l2 = makeLabel();
                emit(JMP, makeString(l2));
            }
            emit(LAB, makeString(l1));
            if (node->child[2] != nullptr) {
                genCode(node->child[2]);
                emit(LAB, makeString(l2));
            }
        }
        void genWhileStmt(ASTNode* node) {
            string l1;
            string l2;
            l1 = makeLabel();
            emit(LAB, makeString(l1));
            genCode(node->child[0]);
            l2 = makeLabel();
            emit(JPC, makeString(l2));
            genCode(node->child[1]);
            emit(JMP, makeString(l1));
            emit(LAB, makeString(l2));
        }
        void genStmt(ASTNode* node) {
            switch (node->type.stmt) {
                case EXPR_STMT: {
                    genCode(node->child[0]);
                } break;
                case PRINT_STMT: {
                    genCode(node->child[0]);
                    emit(PRINT);
                } break;
                case LET_STMT:
                case ASSIGN_STMT: {
                    LocalVar* lv = st.getVar(node->data.strval);
                    emit(LDA, makeInt(lv->loc), makeInt(lv->depth));
                    genCode(node->child[0]);
                    emit(STO);
                } break;
                case IF_STMT: {
                    genIfStmt(node);
                } break;
                case WHILE_STMT: {
                    genWhileStmt(node);
                } break;
                case RETURN_STMT: {
                    genCode(node->child[0]);
                }
                default: break;
            }
        }
        void genBinOp(ASTNode* node) {
            genCode(node->child[0]);
            genCode(node->child[1]);
            switch (node->data.symbol) {
                case TK_ADD: emit(ADD); break;
                case TK_SUB: emit(SUB); break;
                case TK_MUL: emit(MUL); break;
                case TK_DIV: emit(DIV); break;
                default: break;
            }
        }
        void genRelOp(ASTNode* node) {
            genCode(node->child[0]);
            genCode(node->child[1]);
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
        void genExpr(ASTNode* node) {
            switch (node->type.expr) {
                case ID_EXPR: {
                    LocalVar* lv = st.getVar(node->data.strval);
                    emit(genparam ? LDP:LOD, makeInt(lv->loc), makeInt(0));
                } break;
                case CONST_EXPR: {
                    emit(LDC, makeReal(stod(node->data.strval)));
                } break;
                case BINOP_EXPR: {
                    genBinOp(node);
                } break;
                case UNOP_EXPR: {
                    genCode(node->child[0]);
                    emit(NEG);
                } break;
                case RELOP_EXPR: {
                    genRelOp(node);
                } break;
                case STR_EXPR: {
                    emit(LDC, makeString(node->data.strval));
                } break;
                case LAMBDA_EXPR: {
                    string pre = makeLabel();
                    string after = makeLabel();
                    st.openScope(node->data.strval);
                    emit(JMP, makeString(after));
                    emit(ENT, makeString(node->data.strval));
                    genCode(node->child[1]);
                    emit(RET);
                    emit(LAB, makeString(after));
                    st.closeScope();
                } break;
                case FUNC_EXPR: {
                    emit(MST);
                    ASTNode* t = node->child[1];
                    genparam = true;
                    while (t != nullptr) {
                        genCodeNS(t);
                        t = t->next;
                    }
                    genparam = false;
                    emit(CAL, makeString(node->data.strval));
                } break;
                default:
                    break;
            }
        }
        void genCodeNS(ASTNode* node) {
            if (node != nullptr) {
                switch (node->nk) {
                    case EXPR_NODE: genExpr(node); break;
                    case STMT_NODE: genStmt(node); break;
                    default: break;
                }
            }
        }
        void genCodeParam(ASTNode* node) {
            LocalVar* lv = st.getVar(node->data.strval);
            emit(LDP, makeInt(lv->loc), makeInt(0));
        }
        void genCode(ASTNode* node) {
            if (node != nullptr) {
                switch (node->nk) {
                    case EXPR_NODE: genExpr(node); break;
                    case STMT_NODE: genStmt(node); break;
                    default: break;
                }
                genCode(node->next);
            }
        }
        void buildST(ASTNode* node) {
            if (node != nullptr) {
                switch (node->nk) {
                    case STMT_NODE: 
                        switch (node->type.stmt) {
                            case LET_STMT: {
                                if (st.getVar(node->data.strval) == nullptr) {
                                    st.insertVar(node->data.strval);
                                    cout<<node->data.strval<<" added to symbol table"<<endl;
                                }                                
                            } break;
                        };
                        break;
                    case EXPR_NODE: 
                        switch (node->type.expr) {
                            case ID_EXPR: {
                                if (st.getVar(node->data.strval) == nullptr) {
                                    /*st.insertVar(node->data.strval);
                                    cout<<node->data.strval<<" added to symbol table"<<endl;*/
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
        }
    public:
        PCodeGenerator() {
            init();
            genparam = false;
        }
        vector<Instruction> generate(ASTNode* node) {
            init();
            cout<<"Building symbol table"<<endl;
            buildST(node);
            cout<<"Done."<<endl;
            st.print();
            cout<<"Generating P-Code..."<<endl;
            genCode(node);
            emit(HALT);
            codepage.resize(cPos);
            cout<<"Done."<<endl;
            return codepage;
        }
};

#endif