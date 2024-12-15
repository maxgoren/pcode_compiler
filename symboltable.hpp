#ifndef symboltable_hpp
#define symboltable_hpp
#include <iostream>
#include "value.hpp"
using namespace std;

typedef Value Object;

struct Entry {
    string key;
    Object value;
    Entry* next;
    Entry(string k, Object v, Entry* n) : key(k), value(v), next(n) { }
};


class SymbolTable {
    private:
        const static int SIZE = 1013;
        Entry* table[SIZE];
        int hash(string key) {
            int h = 5381;
            for (char c : key) {
                h = (33*h + (int)c);
            }
            return h;
        }
        int count;
        Object nullInfo;
        void print() {
            for (int i = 0; i < SIZE; i++) {
                if (table[i] != nullptr) {
                    for (auto it = table[i]; it != nullptr; it = it->next)
                        cout<<"("<<it->key<<": "<<*toString(it->value)<<"), ";
                    cout<<endl;
                }
            }
        }
    public:
        SymbolTable() {
            nullInfo = makeNil();
            for (int i = 0; i < SIZE; i++)
                table[i] = nullptr;
        }
        void insert(string key, Object value) {
            int idx = hash(key) % SIZE;
            table[idx] = new Entry(key, value, table[idx]);
            count++;
            print();
        }
        Object& get(string key) {
            int idx = hash(key) % SIZE;
            for (auto it = table[idx]; it != nullptr; it = it->next)
                if (key == it->key)
                    return it->value;
            return nullInfo;
        }
        void pop(string key) {
            int idx = hash(key) % SIZE;
            if (table[idx] != nullptr) {
                Entry* t = table[idx];
                table[idx] = table[idx]->next;
                count--;
                delete t;
            }
            print();
        }
        Object& end() {
            return nullInfo;
        }
};



#endif