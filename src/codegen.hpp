#ifndef codegen_hpp
#define codegen_hpp
#include <iostream>
#include <vector>
#include "syntaxtree.hpp"
#include "vminst.hpp"
#include "scoping_st.hpp"
#include <unordered_map>
using namespace std;



class PCodeGenerator {
    private:
        bool should_trace;
        ScopingSymbolTable st;
        string makeLabel() {
            static int labelnum = 0;
            return "L" + to_string(labelnum++);
        }
        vector<Instruction> codepage;
        bool genparam;
        int cPos;
        int highCI;
        void emit(Inst op, Value operand, Value nestLevel) {
            codepage[cPos++] = Instruction(op, operand, nestLevel);
            if (highCI < cPos) highCI = cPos;
        }
        void emit(Inst op, Value operand) {
            codepage[cPos++] = Instruction(op, operand);
            if (highCI < cPos) highCI = cPos;
        }
        void emit(Inst inst) {
            codepage[cPos++] = Instruction(inst);
            if (highCI < cPos) highCI = cPos;
        }
        string emitLabel() {
            string label = makeLabel();
            emit(LAB, makeString(label));
            return label;
        }
        int getLabelAddr(string label) {
            int i = 0;
            while (i < codepage.size()) {
                if (codepage[i].instruction == LAB && label == toStdString(codepage[i].operand))
                    break;
                i++;
            }
            return i;
        }
        int getFunctionAddr(string funcname) {
            int i = 0;
            while (i < codepage.size()) {
                if (codepage[i].instruction == ENT && funcname == toStdString(codepage[i].operand))
                    break;
                i++;
            }
            return i;
        }
        int skipEmit(int spaces) {
            int old = cPos;
            cPos += spaces;
            if (highCI < cPos) highCI = cPos;
            return old;
        } 
        void backup(int addr) {
            cPos = addr;
        }
        void restore() {
            cPos = highCI;
        }
        void genIfStmt(ASTNode* node, bool isAddr) {
            genCode(node->child[0], isAddr);
            int s1 = skipEmit(1);
            genCode(node->child[1], isAddr);
            int s2 = skipEmit(1);
            int c1 = skipEmit(0);
            backup(s1);
            emit(JPC, makeInt(c1));
            restore();
            genCode(node->child[2], isAddr);
            c1 = skipEmit(0);
            backup(s2);
            emit(JMP, makeInt(c1));
            restore();
        }
        void genWhileStmt(ASTNode* node, bool isAddr) {
            string test_label = emitLabel();
            genCode(node->child[0], isAddr);
            int s1 = skipEmit(1);
            genCode(node->child[1], isAddr);
            emit(JMP, makeInt(getLabelAddr(test_label)));
            //string exit_label = emitLabel();
            int c1 = skipEmit(0);
            backup(s1);
            emit(JPC, makeInt(c1)); //getLabelAddr(exit_label)));
            restore();
        }
        void genStmt(ASTNode* node, bool isAddr) {
            switch (node->type.stmt) {
                case PROGRAM_STMT: {
                    genCode(node->child[0], isAddr);
                } break;
                case EXPR_STMT: {
                    genCode(node->child[0], isAddr);
                } break;
                case PRINT_STMT: {
                    genCode(node->child[0], isAddr);
                    emit(PRINT);
                } break;
                case REF_STMT: {
                    LocalVar* lv = st.getVar(node->data.strval);
                    emit(LDA, makeInt(lv->loc), makeInt(lv->depth));
                    genparam = true;
                    genCodeNS(node->child[0],true);
                    genparam = false;
                    emit(STN);
                }  break;
                case LET_STMT: {
                    LocalVar* lv = st.getVar(node->data.strval);
                    emit(LDA, makeInt(lv->loc), makeInt(lv->depth));
                    genCodeNS(node->child[0],false);
                    emit(STN);
                } break;
                case FUNC_DEF_STMT: {
                    st.openScope(node->data.strval);
                    int s1 = skipEmit(2);
                    emit(ENT, makeString(node->data.strval));
                    genCode(node->child[1], isAddr);
                    emit(RET);
                    int c1 = skipEmit(0);
                    backup(s1);
                    emit(JMP, makeInt(c1));
                    string pre = makeLabel();
                    emit(LAB, makeString(pre));
                    restore();
                    st.closeScope();
                } break;
                case IF_STMT: {
                    genIfStmt(node, isAddr);
                } break;
                case WHILE_STMT: {
                    genWhileStmt(node, isAddr);
                } break;
                case RETURN_STMT: {
                    genCode(node->child[0], isAddr);
                } break;
                case BLOCK_STMT: {
                    st.openScope(node->data.strval);
                    emit(MST);
                    emit(ENT);
                    genCode(node->child[0], false);
                    emit(RET);
                    st.closeScope();
                } break;
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
                    } else if ((isAddr && !genparam) || hasSubscript(node) || hasField(node)) {
                        emit(LDA, makeInt(lv->loc), makeInt(0));
                    } else {
                        if (genparam) {
                            emit(isAddr ? LRP:LDP, makeInt(lv->loc), makeInt(0));
                        } else {
                            emit(LOD, makeInt(lv->loc), makeInt(0));
                        }
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
                    genCodeNS(node->child[LEFTCHILD], false);
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
                    switch (node->data.symbol) {
                        case TK_SUB: emit(NEG); break;
                        case TK_NOT: emit(NOT); break;
                        default: break;
                    }
                } break;
                case RELOP_EXPR: {
                    genRelOp(node, isAddr);
                } break;
                case STR_EXPR: {
                    emit(LDC, makeString(node->data.strval));
                } break;
                case ASSIGN_EXPR: {
                    genExpr(node->child[LEFTCHILD], true);
                    genExpr(node->child[RIGHTCHILD], false);
                    emit(STO);
                } break;
                case FUNC_EXPR: {
                    emit(MST);
                    ASTNode* t = node->child[1];
                    genparam = true;
                    int sloc = 0;
                    while (t != nullptr) {
                        genCodeNS(t, isAddr);
                        t = t->next;
                        sloc++;
                    }
                    genparam = false;
                    emit(CAL, makeInt(getFunctionAddr(node->data.strval)));
                } break;
                case BLESS_EXPR: {
                    int saddr = st.allocStruct(node->child[LEFTCHILD]->data.strval);
                    emit(LDA, makeInt(saddr));
                } break;
                case REG_EXPR: {
                    genCode(node->child[0], false);
                    genCode(node->child[1], false);
                    emit(MATCHRE);
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
        string makeScopeLabel() {
            static int labelnum = 0;
            labelnum++;
            return "blockscope" + to_string(labelnum);
        } 
        void buildST(ASTNode* node) {
            if (node != nullptr) {
                switch (node->nk) {
                    case STMT_NODE: 
                        switch (node->type.stmt) {
                            case REF_STMT:
                            case LET_STMT: {
                                if (hasSubscript(node)) {
                                    st.insertVar(node->data.strval, atoi(node->child[0]->data.strval.data()));
                                    if (should_trace)
                                        cout<<node->data.strval<<" added to symbol table as an array of size "<<atoi(node->child[0]->data.strval.data())<<endl;
                                } else {
                                    st.insertVar(node->data.strval);
                                    if (node->child[0] != nullptr && node->child[0]->type.expr == BLESS_EXPR) {
                                        st.addInstance(node->data.strval, node->child[0]->child[0]->data.strval);
                                    }
                                    if (should_trace)
                                        cout<<node->data.strval<<" added to symbol table"<<endl;
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
                            case BLOCK_STMT: {
                                node->data.strval = makeScopeLabel();
                                st.openScope(node->data.strval);
                                buildST(node->child[0]);
                                st.closeScope();
                            } break;
                            case FUNC_DEF_STMT: {
                                st.openScope(node->data.strval);
                                buildST(node->child[0]);
                                buildST(node->child[1]);
                                st.closeScope();
                                buildST(node->next);
                                return;
                            } break;
                        };
                        break;
                    case EXPR_NODE: {
                        switch (node->type.expr) {
                            case ID_EXPR: {
                                if (hasField(node)) {
                                    ASTNode* t = node->child[0];
                                    Scope* ts = st.getStruct(st.getType(node->data.strval));
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
                        };
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
            if (cPos > 0) cPos--;
            if (should_trace)
                cout<<"Building Symbol Table: "<<endl;
            buildST(node);
            if (should_trace ) {
                st.print();
                cout<<"Generating P-Code..."<<endl;
            }
            genCode(node, false);
            emit(HALT);
            if (should_trace)
                cout<<"Done."<<endl;
            return codepage;
        }
};


#endif