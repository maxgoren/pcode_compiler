#ifndef treewalk_interp_hpp
#define treewalk_interp_hpp
#include "syntaxtree.hpp"
#include "symboltable.hpp"
#include "value.hpp"
using namespace std;

int d = 0;
void say(string s) {
    for (int i = 0; i < d; i++) cout<<" ";
    cout<<s<<endl;
}

void enter(string s) {
    d++;
    say(s);
}

void leave() {
    d--;
}

struct Context {
    SymbolTable st;
    Context* parent;
    Context() {
        parent = nullptr;
    }
};

Context* scopeSymbolTable(Context* cxt) {
    Context* t = new Context();
    t->parent = cxt;
    return t;
}

Context* unscopeSymbolTable(Context* cxt) {
    if (cxt != nullptr && cxt->parent != nullptr) {
        Context* t = cxt;
        cxt = cxt->parent;
        delete t;
    }
    return cxt;
}

Value getSymbol(Context* cxt, string key) {
    if (cxt == nullptr)
        return makeNil();
    Value t = cxt->st.get(key);
    if (t.type != AS_NIL) 
        return t;
    return getSymbol(cxt->parent, key); 
}

class TreeWalkingInterpreter {
    private:
        Context* cxt;
        Value evalBinop(Symbol op, Value lhs, Value rhs);
        Value evalRelop(Symbol op, Value lhs, Value rhs);
        Value eval(ASTNode* node);
        Value execFunc(Function* func, ASTNode* args);
        Value funcCall(ASTNode* node);
        Value lambdaExpr(ASTNode* node);
        Value execExpr(ASTNode* node);
        Value execStmt(ASTNode* node);
    public:
        TreeWalkingInterpreter();
        Value execute(ASTNode* node);
};

TreeWalkingInterpreter::TreeWalkingInterpreter() {
    cxt = new Context();
}

Value TreeWalkingInterpreter::evalBinop(Symbol op, Value lhs, Value rhs) {
    enter("eval binop");
    leave();
    switch (op) {
        case TK_ADD: return add(lhs, rhs);
        case TK_SUB: return sub(lhs, rhs);
        case TK_MUL: return mul(lhs, rhs);
        case TK_DIV: return div(lhs, rhs);
        default: break;
    }
    return makeInt(0);
}

Value TreeWalkingInterpreter::evalRelop(Symbol op, Value lhs, Value rhs) {
    enter("eval rel op");
    leave();
    switch (op) {
        case TK_LT: return lt(lhs, rhs);
        case TK_GT: return gt(lhs, rhs);
        case TK_LTE: return lte(lhs, rhs);
        case TK_GTE: return gte(lhs, rhs);
        case TK_EQU: return equ(lhs, rhs);
        case TK_NEQ: return neq(lhs, rhs);
        default: break;
    }
    return makeBool(false);
}

Value TreeWalkingInterpreter::eval(ASTNode* node) {
    enter("eval");
    if (node != nullptr) {
        Value lhs = execExpr(node->child[0]), rhs = execExpr(node->child[1]);
        if (node->type.expr == BINOP_EXPR) {
            if ((lhs.type == AS_STRING && rhs.type == AS_STRING)) {
                if (node->data.symbol == TK_ADD) {
                    return concatStrings(lhs, rhs);
                } else {
                    cout<<symbolStr[node->data.symbol]<<" not supported on strings."<<endl;
                    return makeInt(0);
                }
            }
            leave();
            return evalBinop(node->data.symbol, lhs, rhs);
        } else {
            leave();
            return evalRelop(node->data.symbol, lhs, rhs);
        }   
    }
    leave();
    return makeInt(0);
}

Value TreeWalkingInterpreter::execFunc(Function* func, ASTNode* args) {
    enter("exec function");
    ASTNode* aItr = args;
    ASTNode* pItr = func->params;
    cxt = scopeSymbolTable(cxt);
    while (pItr != nullptr && aItr != nullptr) {
        string vname = pItr->data.strval;
        cxt->st.insert(vname, execExpr(aItr));
        pItr = pItr->next;
        aItr = aItr->next;
    }
    Value val = execute(func->body);
    cxt = unscopeSymbolTable(cxt);
    leave();
    return val;
}

Value TreeWalkingInterpreter::funcCall(ASTNode* node) {
    Value func = execExpr(node->child[0]);
    return execFunc(func.funcval, node->child[1]);
}

Value TreeWalkingInterpreter::lambdaExpr(ASTNode* node) {
    Function* func = new Function;
    func->params = node->child[0];
    func->body = node->child[1];
    return makeFunction(func);
}

Value TreeWalkingInterpreter::execExpr(ASTNode* node) {
    enter("expr");
    leave();
    switch (node->type.expr) {
        case CONST_EXPR:  return makeReal(stod(node->data.strval));
        case ID_EXPR:     return getSymbol(cxt, node->data.strval);
        case STR_EXPR:    return makeString(node->data.strval);
        case UNOP_EXPR:   return neg(execExpr(node->child[0]));
        case LAMBDA_EXPR: return lambdaExpr(node);
        case FUNC_EXPR:   return funcCall(node);
        case RELOP_EXPR:
        case BINOP_EXPR:  return eval(node);
        default: break;
    };
    return makeInt(0);
}

Value TreeWalkingInterpreter::execStmt(ASTNode* node) {
    enter("stmt");
    if (node == nullptr || node->nk != STMT_NODE)
        return makeNil();
    switch (node->type.stmt) {
        case PRINT_STMT: {
            cout<<*toString(execExpr(node->child[0]))<<endl;
        } break;
        case LET_STMT:
        case ASSIGN_STMT: {
            cxt->st.insert(node->data.strval, execExpr(node->child[1]));
        } break;
        case EXPR_STMT: {
            Value result = execExpr(node->child[0]);
            cout<<"Result: "<<*toString(result)<<endl;
            leave();
            return result;
        } break;
        case WHILE_STMT: {
            cout<<"Dem all get funky"<<endl;
            while (execExpr(node->child[0]).boolval) {
                execute(node->child[1]);
            }
        } break;
        case IF_STMT: {
            Value result;
            if (execExpr(node->child[0]).boolval) {
                result = execute(node->child[1]);
            } else {
                result = execute(node->child[2]);
            }
            return result;
        } break;
        case RETURN_STMT: {
            leave();
            return execute(node->child[0]);
        }
        default:
            break;
    }
    leave();
    return makeNil();
}

Value TreeWalkingInterpreter::execute(ASTNode* node) {
    enter("execute");
    Value v;
    if (node != nullptr) {
        switch (node->nk) {
            case EXPR_NODE: v = execExpr(node); break;
            case STMT_NODE: v = execStmt(node); break;
            default: break;
        };
        if (node->next != nullptr)
            v = execute(node->next);
    }
    leave();
    return v;
}

#endif