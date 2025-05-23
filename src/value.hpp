#ifndef value_hpp
#define value_hpp
#include <iostream>
#include <cstring>
#include <cmath>
#include "syntaxtree.hpp"
using namespace std;

enum ValueType {
    AS_INT, AS_BOOL, AS_REAL, AS_STRING, AS_FUNC, AS_NIL, AS_ARRDEF
};

struct String {
    char* str;
    int len;
};

String* createString(const char* str, int len) {
    String* ns = new String;
    ns->str = new char[len];
    int i;
    for (i = 0; str[i]; i++) {
        ns->str[i] = str[i];
    }
    ns->str[i] = '\0';
    ns->len = len;
    return ns;
}

std::ostream& operator<<(std::ostream& os, String& strObj) {
    for (int i = 0; i < strObj.len; i++)
        os<<strObj.str[i];
    return os;
}

struct Function {
    string name;
    int ip;
    int returnAddr;
    int numArgs;
    int numLocals;
    Function(string n = "", int i = 0, int ra = 0, int na = 0, int nl = 0) : 
    name(n), ip(i), returnAddr(ra), numArgs(na), numLocals(nl) { }
};

struct Value {
    ValueType type;
    union {
        String* strval;
        int intval;
        double realval;
        bool boolval;
        Function* funcval;
    };
};

bool isRealAnInteger(double val) {
    string num = to_string(val);
    int i = 0;
    while (i < num.size() && num[i++] != '.');
    while (i < num.size() && num[i++] == '0');
    return i == num.size();
}

Value makeInt(int n) {
    Value nv;
    nv.type = AS_INT;
    nv.intval = n;
    return nv;
}

Value makeBool(bool val) {
    Value nv;
    nv.type = AS_BOOL;
    nv.boolval = val;
    return nv;
}

bool isWhole(double val) {
    return std::floor(val) == val;
}

Value makeReal(double val) {
    if (isWhole(val)) {
        return makeInt((int)val);
    }
    Value nv;
    nv.type = AS_REAL;
    nv.realval = val;
    return nv;
}
Value makeNil() {
    Value nv;
    nv.type = AS_NIL;
    nv.intval = -1;
    return nv;
}
Value makeFunction(Function* func) {
    Value nv;
    nv.type = AS_FUNC;
    nv.funcval = func;
    return nv;
}

Value makeString(string str) {
    Value nv;
    nv.type = AS_STRING;
    nv.strval = createString(str.data(), str.length());
    return nv;
}

Value makeString(String* str) {
    Value nv;
    nv.type = AS_STRING;
    nv.strval = str;
    return nv;
}

Value concatStrings(Value lhs, Value rhs) {
    String* lstr = lhs.strval;
    String* rstr = rhs.strval;
    char* tmp = new  char[lstr->len + rstr->len];
    int k = 0, i = 0;
    while (k < lstr->len) { tmp[k] = lstr->str[k]; k++; }
    while (i < rstr->len) { tmp[k++] = rstr->str[i++]; }
    String* str = new String;
    str->str = tmp;
    str->len = k;
    return makeString(str);
}

Value repeatString(Value strVal, int numRepeat) {
    string tmp;
    for (int i = 0; i < numRepeat; i++) {
        for (int j = 0; j < strVal.strval->len; j++) {
            tmp.push_back(strVal.strval->str[j]);
        }
    }
    return makeString(tmp);
}

bool compareStrings(String* lhs, String* rhs) {
    if (lhs->len != rhs->len)
        return false;
    int i = 0, j = 0;
    while (i < lhs->len && j < rhs->len) {
        if (lhs->str[i] != rhs->str[j]) {
            return false;
        }
        i++; j++;
    }
    return i == j;
}

String* toString(Value val) {
    switch (val.type) {
        case AS_REAL:   {
            string num = std::to_string(val.realval);
            return createString(num.data(), num.length());
        }
        case AS_BOOL:   {
            string num = val.boolval ? "true":"false";
            return createString(num.data(), num.length());
        }
        case AS_INT:    {
            string num = std::to_string(val.intval);
            return createString(num.data(), num.length());
        }
        case AS_FUNC:   {
            string val = "(lambda)";
            return createString(val.data(), val.length());
        }
        case AS_NIL:    {
            string val = "(nil)";
            return createString(val.data(), val.length());
        }
        case AS_STRING: return val.strval;
    }
    return createString(" ", 1);
}

std::string toStdString(Value obj) {
    String* str = toString(obj);
    return string(str->str, str->len);
}

std::ostream& operator<<(std::ostream& os, Value& val) {
    os<<*toString(val);
    return os;
}

double getReal(Value val) {
    return val.realval;
}

bool getBoolean(Value val) {
    return val.boolval;
}

int getInteger(Value val) {
    return val.intval;
}

Function* getFunction(Value val) {
    return val.funcval;
}

String* getString(Value val) {
    return val.strval;
}

bool isNull(Value val) {
    return val.type == AS_NIL;
}

double getPrimitive(Value lhs) {
    double a = 0;
    switch (lhs.type) {
        case AS_INT: a = lhs.intval; break;
        case AS_REAL: a = lhs.realval; break;
        case AS_BOOL: a = lhs.boolval; break;
    }
    return a;
}

std::pair<double, double> getPrimVals(Value lhs, Value rhs) {
    double a = getPrimitive(lhs);
    double b = getPrimitive(rhs);
    return make_pair(a, b);
}

bool isZero(Value val) {
    switch (val.type) {
        case AS_BOOL: return val.boolval == false;
        case AS_INT: return val.intval == 0;
        case AS_REAL: return val.realval == 0.0;
    }
    return false;
}

Value Add(Value lhs, Value rhs) {
    if (lhs.type == AS_STRING || rhs.type == AS_STRING)
        return concatStrings(makeString(toString(lhs)), makeString(toString(rhs)));
    if ((lhs.type == AS_INT || lhs.type == AS_REAL) && (rhs.type == AS_INT || rhs.type == AS_REAL)) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeReal(a + b);
    }
    return makeInt(0);    
}

Value Sub(Value lhs, Value rhs) {
    if ((lhs.type == AS_INT || lhs.type == AS_REAL) && (rhs.type == AS_INT || rhs.type == AS_REAL)) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeReal(a - b);
    }    
    return makeInt(0);    
}

Value Mul(Value lhs, Value rhs) {
    if (lhs.type == AS_STRING || rhs.type == AS_STRING) {
        if (lhs.type == AS_STRING) {
            return repeatString(lhs, getPrimitive(rhs));
        } else {
            return repeatString(rhs, getPrimitive(lhs));
        }
    }
    if ((lhs.type == AS_INT || lhs.type == AS_REAL) && (rhs.type == AS_INT || rhs.type == AS_REAL)) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeReal(a * b);
    }    
    return makeInt(0);    
}

Value Div(Value lhs, Value rhs) {
    if ((lhs.type == AS_INT || lhs.type == AS_REAL) && (rhs.type == AS_INT || rhs.type == AS_REAL)) {
        auto [a, b] = getPrimVals(lhs, rhs);
        if (b == 0) {
            cout<<"Error: divide by zero"<<endl;
            return makeInt(0);
        }
        return makeReal(a / b);
    }    
    return makeInt(0);    
}

Value equ(Value lhs, Value rhs) {
    if ((lhs.type == AS_INT || lhs.type == AS_REAL) && (rhs.type == AS_INT || rhs.type == AS_REAL)) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeBool(a == b);
    }
    return makeBool(toStdString(lhs) == toStdString(rhs));    
}

Value neq(Value lhs, Value rhs) {
    if ((lhs.type == AS_INT || lhs.type == AS_REAL) && (rhs.type == AS_INT || rhs.type == AS_REAL)) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeBool(a != b);
    }    
    return makeBool(toStdString(lhs) != toStdString(rhs));    
}

Value lte(Value lhs, Value rhs) {
    if ((lhs.type == AS_INT || lhs.type == AS_REAL) && (rhs.type == AS_INT || rhs.type == AS_REAL)) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeBool(a <= b);
    }    
    return makeBool(toStdString(lhs) <= toStdString(rhs));    
}

Value gte(Value lhs, Value rhs) {
    if ((lhs.type == AS_INT || lhs.type == AS_REAL) && (rhs.type == AS_INT || rhs.type == AS_REAL)) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeBool(a >= b);
    }    
    return makeBool(toStdString(lhs) >= toStdString(rhs));    
}

Value lt(Value lhs, Value rhs) {
    if ((lhs.type == AS_INT || lhs.type == AS_REAL) && (rhs.type == AS_INT || rhs.type == AS_REAL)) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeBool(a < b);
    }    
    return makeBool(toStdString(lhs) < toStdString(rhs));    
}

Value gt(Value lhs, Value rhs) {
    if ((lhs.type == AS_INT || lhs.type == AS_REAL) && (rhs.type == AS_INT || rhs.type == AS_REAL)) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeBool(a > b);
    }    
    return makeBool(toStdString(lhs) > toStdString(rhs));    
}

Value Neg(Value lhs) {
    if (lhs.type == AS_INT || lhs.type == AS_REAL || lhs.type == AS_BOOL) {
        double a = 0;
        switch (lhs.type) {
            case AS_INT: a = lhs.intval; break;
            case AS_REAL: a = lhs.realval; break;
            case AS_BOOL: a = lhs.boolval; break;
        }
        return makeReal(-a);
    }
    return makeInt(0);
}

Value Not(Value lhs) {
    if (lhs.type == AS_BOOL) {
        bool a;
        switch (lhs.type) {
            case AS_BOOL: a = lhs.boolval; break;
        }
        return makeBool(!a);
    }
    return makeInt(0);
}
#endif