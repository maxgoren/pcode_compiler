#ifndef scoping_st_hpp
#define scoping_st_hpp
#include <iostream>
using namespace std;


const int TABLE_SIZE = 211;

int hashf(string key) {
    unsigned int h = 5781;
    for (char c : key) {
        h = (33 * h + (int)c);
    }
    return h % TABLE_SIZE;
}

enum DefType {
    VARDEF, PROCDEF, EMPTY
};

struct LocalVar {
    int loc;
    int depth;
    LocalVar(int l = 0, int d = 0) : loc(l), depth(d) { }
};

struct Scope;

struct STEntry {
    string name;
    DefType type;
    union {
        LocalVar* localvar;
        Scope* procedure;
    };
    STEntry* next;
    STEntry(string n = "") : name(n), next(nullptr) { }
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
                        cout<<"localvar: "<<x->table[i]->localvar->loc<<endl;
                    }
                    if (x->table[i]->type == PROCDEF) {
                        cout<<"Procedure: "<<x->table[i]->name<<endl;
                        dump(x->table[i]->procedure, sd+1);
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
            localAddr = 3000;
        }
        void insertVar(string name) {
            int idx = hashf(name);
            for (STEntry* it = scope->table[idx]; it != nullptr; it = it->next) {
                if (it->name == name)
                    return;
            }
            STEntry* nent = makeLocalVarEntry(name, scopeIsGlobal() ? --localAddr:scope->numEntries++, scopeDepth);
            nent->next = scope->table[idx]; 
            scope->table[idx] = nent;
        }
        LocalVar* getVar(string name) {
            STEntry* ent = get(name, VARDEF);
            return ent->type == VARDEF ? ent->localvar:nullptr;
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
        void print() {
            cout<<"Symbol Table: "<<endl;
            dump(scope, 0);
        }
};

#endif