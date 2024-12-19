#ifndef scoping_st_hpp
#define scoping_st_hpp
#include <iostream>
#include "syntaxtree.hpp"
using namespace std;


const int TABLE_SIZE = 256;

int hashf(string key) {
    unsigned int h = 5781;
    for (char c : key) {
        h = (33 * h + (int)c);
    }
    return h & (TABLE_SIZE-1);
}

enum DefType {
    VARDEF, PROCDEF, STRUCTDEF, EMPTY
};

enum VarDefType {
    SCALAR, ARRAY
};

struct LocalVar {
    VarDefType type;
    int loc;
    int depth;
    int size;
    LocalVar(int l = 0, int d = 0) : type(SCALAR), loc(l), depth(d), size(1) { }
};

struct Scope;

struct STEntry {
    string name;
    int addr;
    DefType type;
    union {
        LocalVar* localvar;
        Scope* procedure;
        Scope* structure;
    };
    STEntry* next;
    STEntry(string n = "") : name(n), next(nullptr), addr(0) { }
};

struct Scope {
    int numEntries;
    STEntry* table[TABLE_SIZE];
    Scope* enclosing;
    Scope() {
        numEntries = 0;
        enclosing = nullptr;
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
    }
};

STEntry* makeLocalVarEntry(string name, int location, int depth) {
    STEntry* ent = new STEntry(name);
    ent->type = VARDEF;
    ent->localvar = new LocalVar(location, depth);
    return ent;
}

STEntry* makeProcedureEntry(string name, Scope* ns) {
    STEntry* ent = new STEntry(name);
    ent->type = PROCDEF;
    ent->procedure = ns;
    return ent;
}

STEntry* makeStructEntry(string name, Scope* ns, int addr) {
    STEntry* ent = new STEntry(name);
    ent->type = STRUCTDEF;
    ent->structure = ns;
    ent->addr = addr;
    return ent;
}

STEntry* makeEmptyEntry() {
    STEntry* ent = new STEntry("<empty>");
    ent->type = EMPTY;
    ent->localvar = nullptr;
    return ent;
}

class ScopingSymbolTable {
    private:
        Scope* scope;
        int scopeDepth;
        int localAddr;
        STEntry* get(string name, DefType type) {
            cout<<"Searching for: "<<name<<" ";
            Scope* x = scope;
            int idx = hashf(name);
            while (x != nullptr) {
                 cout<<" . ";
                for (STEntry* it = x->table[idx]; it != nullptr; it = it->next) {
                    if (it->name == name) {
                        cout<<"Found."<<endl;
                        return it;
                    }
                }
                x = x->enclosing;
            }
            cout<<"Not found."<<endl;
            return makeEmptyEntry();
        }
        void dump(Scope* s, int sd) {
            if (s == nullptr) return;
            Scope* x = s;
            for (int i = 0; i < TABLE_SIZE; i++) {
                if (x->table[i] != nullptr) {
                    for (int i = 0; i < sd; i++) {
                        cout<<"\t\t\t";
                    }
                    cout<<x->table[i]->name<<": ";
                    if (x->table[i]->type == VARDEF) {
                        cout<<"Localvar: "<<x->table[i]->localvar->loc<<", ";
                        switch (x->table[i]->localvar->type) {
                            case SCALAR: {
                                cout<<"Scalar."<<endl;
                            } break;
                            case ARRAY: {
                                cout<<"Array, size: "<<x->table[i]->localvar->size<<endl;
                            } break;
                        }
                    }
                    if (x->table[i]->type == PROCDEF) {
                        cout<<"Procedure: "<<x->table[i]->name<<endl;
                        dump(x->table[i]->procedure, sd+1);
                    }
                    if (x->table[i]->type == STRUCTDEF) {
                        cout<<"Struct: "<<x->table[i]->name<<", "<<x->table[i]->addr<<endl;
                        dump(x->table[i]->structure, sd+1);
                    }
                }
            }
        }
        bool scopeIsGlobal() {
            return scope->enclosing == nullptr;
        }
    public:
        ScopingSymbolTable() {
            scope = new Scope();
            scope->enclosing = nullptr;
            scopeDepth = 0;
            localAddr = 5000;
        }
        void insertVar(string name, int size) {
            int idx = hashf(name);
            for (STEntry* it = scope->table[idx]; it != nullptr; it = it->next) {
                if (it->name == name)
                    return;
            }
            int addr = 0;
            if (scopeIsGlobal()) {
                addr = localAddr;
                localAddr -= size;
            } else {
                if (size == 1) {
                    addr = scope->numEntries;
                } else {
                    addr = scope->numEntries + size;
                }
                scope->numEntries += size;
            }
            STEntry* nent = makeLocalVarEntry(name, addr, scopeDepth);
            if (size > 1) { 
                nent->localvar->type = ARRAY;
                nent->localvar->size = size;
            }
            nent->next = scope->table[idx]; 
            scope->table[idx] = nent;
        }
        void insertVar(string name) {
            insertVar(name, 1);
        }
        LocalVar* getVar(string name) {
            STEntry* ent = get(name, VARDEF);
            if (ent->type == VARDEF) {
                return ent->localvar;
            } else if (ent->type == STRUCTDEF) {
                return new LocalVar(ent->addr, 0);
            }
            return nullptr;
        }
        Scope* insertProc(string name) {
            int idx = hashf(name);
            for (STEntry* it = scope->table[idx]; it != nullptr; it = it->next) {
                if (it->name == name)
                    return it->procedure;
            }
            Scope* ns = new Scope();
            ns->enclosing = scope;
            STEntry* nent = makeProcedureEntry(name, ns);
            nent->next = scope->table[idx]; 
            scope->table[idx] = nent;
            return ns;
        }
        Scope* getProc(string name) {
            STEntry* ent = get(name, PROCDEF);
            return ent->type == PROCDEF ? ent->procedure:nullptr;
        }
        void openScope(string name) {
            Scope* st = getProc(name);
            if (st == nullptr) {
                st = insertProc(name);
            }
            st->enclosing = scope;
            scope = st;
            scopeDepth++;
            cout<<"Open scope for: "<<name<<endl;
        }
        void closeScope() {
            if (scope->enclosing != nullptr) {
                scope = scope->enclosing;
                scopeDepth--;
                cout<<"Closing scope."<<endl;
            }
        }
        Scope* insertStruct(string name, int size) {
            int idx = hashf(name);
            for (STEntry* it = scope->table[idx]; it != nullptr; it = it->next) {
                if (it->name == name)
                    return it->structure;
            }
            Scope* ns = new Scope();
            ns->enclosing = scope;
            int addr = localAddr;
            localAddr -= size;
            STEntry* nent = makeStructEntry(name, ns, addr);
            nent->next = scope->table[idx]; 
            scope->table[idx] = nent;
            return ns;
        }
        Scope* getStruct(string name) {
            STEntry* ent = get(name, STRUCTDEF);
            return ent->type == STRUCTDEF ? ent->structure:nullptr;
        }
        LocalVar* getFieldFromStruct(Scope* stScope, string fieldname) {
            if (stScope == nullptr)
                return nullptr;
            int idx = hashf(fieldname);
            for (STEntry* it = stScope->table[idx]; it != nullptr; it = it->next) {
                if (it->name == fieldname) {
                    cout<<"Found."<<endl;
                    return it->localvar;
                }
            }
            return nullptr;
        }
        void openStruct(ASTNode* node) {
            string name = node->data.strval;
            int size = 1;
            auto t = node->child[0];
            while (t != nullptr) {
                size++;
                t = t->next;
            }
            Scope* st = getStruct(name);
            if (st == nullptr) {
                st = insertStruct(name, size);
            }
            st->enclosing = scope;
            scope = st;
            scopeDepth++;
            cout<<"Open struct scope for: "<<name<<endl;
        }
        void closeStruct() {
            if (scope->enclosing != nullptr) {
                scope = scope->enclosing;
                scopeDepth--;
                cout<<"Closing struct scope."<<endl;
            }
        }
        void print() {
            cout<<"Symbol Table: "<<endl;
            dump(scope, 0);
        }
};

#endif