#ifndef value_hpp
#define value_hpp
#include <iostream>
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
    for (int i = 0; i < len; i++) {
        ns->str[i] = str[i];
    }
    ns->str[len] = '\0';
    ns->len = len;
    return ns;
}

std::ostream& operator<<(std::ostream& os, String& strObj) {
    os<<strObj.str;
    return os;
}

struct Function {
    ASTNode* body;
    ASTNode* params;
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

Value makeReal(double val) {
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
    int k = 0;
    while (k < lstr->len) {
        tmp[k] = lstr->str[k];
        k++;
    }
    int i = 0;
    while (i < rstr->len) {
        tmp[k] = rstr->str[i];
        k++; i++;
    }
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

Value add(Value lhs, Value rhs) {
    if (lhs.type == AS_STRING || rhs.type == AS_STRING)
        return concatStrings(makeString(toString(lhs)), makeString(toString(rhs)));
    if (lhs.type == AS_INT || lhs.type == AS_REAL) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeReal(a + b);
    }
    return makeInt(0);    
}

Value sub(Value lhs, Value rhs) {
    if (lhs.type == AS_INT || lhs.type == AS_REAL) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeReal(a - b);
    }    
    return makeInt(0);    
}

Value mul(Value lhs, Value rhs) {
    if (lhs.type == AS_STRING || rhs.type == AS_STRING) {
        if (lhs.type == AS_STRING) {
            return repeatString(lhs, getPrimitive(rhs));
        } else {
            return repeatString(rhs, getPrimitive(lhs));
        }
    }
    if (lhs.type == AS_INT || lhs.type == AS_REAL) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeReal(a * b);
    }    
    return makeInt(0);    
}

Value div(Value lhs, Value rhs) {
    if (lhs.type == AS_INT || lhs.type == AS_REAL) {
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
    if (lhs.type == AS_INT || lhs.type == AS_REAL) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeBool(a == b);
    }
    return makeInt(0);        
}

Value neq(Value lhs, Value rhs) {
    if (lhs.type == AS_INT || lhs.type == AS_REAL) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeBool(a != b);
    }    
    return makeInt(0);    
}

Value lte(Value lhs, Value rhs) {
    if (lhs.type == AS_INT || lhs.type == AS_REAL) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeBool(a <= b);
    }    
    return makeInt(0);    
}

Value gte(Value lhs, Value rhs) {
    if (lhs.type == AS_INT || lhs.type == AS_REAL) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeBool(a >= b);
    }    
    return makeInt(0);    
}

Value lt(Value lhs, Value rhs) {
    if (lhs.type == AS_INT || lhs.type == AS_REAL) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeBool(a < b);
    }    
    return makeInt(0);    
}

Value gt(Value lhs, Value rhs) {
    if (lhs.type == AS_INT || lhs.type == AS_REAL) {
        auto [a, b] = getPrimVals(lhs, rhs);
        return makeBool(a > b);
    }    
    return makeInt(0);    
}

Value neg(Value lhs) {
    if (lhs.type == AS_INT || lhs.type == AS_REAL) {
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

#endif