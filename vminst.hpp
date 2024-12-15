#ifndef vminst_hpp
#define vminst_hpp
#include <iomanip>
#include "value.hpp"
using namespace std;

enum Inst {
    LDC, LDA, LOD, LDP,
    STO, STN, STP,
    LAB, MST, ENT, CAL, 
    RET, JMP, JPC,
    NEG, ADD, SUB,
    MUL, DIV, MOD,
    EQU, NEQ, LTE, GTE,
    LT, GT, TS, INC, DEC, 
    PRINT, HALT
};

string instStr[] = {
    "LDC", "LDA", "LOD", 
    "LDP",
    "STO", "STN", "STP",
    "LAB",
    "MST", "ENT", "CAL", 
    "RET", "JMP", "JPC",
    "NEG", "ADD", "SUB",
    "MUL", "DIV", "MOD",
    "EQU","NEQ", "LTE", 
    "GTE", "LT", "GT", 
    "TS", "INC", "DEC", 
    "PRINT", 
    
    "HALT"
};

struct Instruction {
    Inst instruction;
    Value operand;
    Value nestlevel;
    Instruction(Inst i = HALT, Value a = makeInt(0) , Value b = makeInt(0)) : instruction(i), operand(a), nestlevel(b) { }    
};

std::ostream& operator<<(std::ostream& os, const Instruction& inst) {
    os<<"("<<setw(4)<<instStr[inst.instruction]<<", "<<*toString(inst.operand)<<", "<<*toString(inst.nestlevel)<<")";
    return os;
}

#endif