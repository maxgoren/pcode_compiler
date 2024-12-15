#ifndef pmachine_hpp
#define pmachine_hpp
#include <iostream>
#include <vector>
#include "value.hpp"
#include "vminst.hpp"
using namespace std;


class PCodeVM {
    private:
        bool should_trace;
        const int MAX_STACK = 3000;
        vector<Instruction> codePage;
        vector<Value> stack;
        Instruction curr;
        int sp;
        int ip;
        int bp;
        int lv;
        Instruction current() {
            return curr;
        }
        int base(int lvl) {
            int np = bp;
            while (lvl > 0) {
                np = getInteger(stack[np+1]);
                lvl--;
            }
            return np+4;
        }
        int calculateAddress(int offset) {
            int bn = 0;
            if (offset < 2000)
                bn = base(getInteger(current().nestlevel));
            if (should_trace) {
                cout<<"Relative Addr: "<<offset<<endl;
                cout<<"Calculated bn: "<<bn<<endl;
                cout<<"Calculated ad: "<<bn+offset<<endl;
            }
            return bn+offset;
        }
        int findLabel(String* str, Inst instType) {
            for (int i = 0; i < MAX_STACK; i++) {
                if (i != ip && codePage[i].instruction == instType) {
                    if (compareStrings(str, getString(codePage[i].operand))) {
                        return i;
                    }
                }
            }
            return ip;
        }
        void nextInstruction() {
            curr = codePage[ip];
            ip++;
            if (should_trace)
                cout<<"Executing: "<<ip<<": "<<instStr[current().instruction]<<" "<<*toString(current().operand)<<" "<<*toString(current().nestlevel)<<endl;
        }
    public:
        PCodeVM(bool trace = false) {
            ip = 0;
            bp = 1;
            sp = bp;
            lv = 0;
            stack.reserve(MAX_STACK);
            codePage.reserve(MAX_STACK);
            should_trace = trace;
        }
        void setTrace(bool trace) {
            should_trace = trace;
        }
        void init(vector<Instruction>& code) {
            for (int i = 0, j = ip; i < code.size(); i++, j++)
                codePage[j] = code[i];
            curr = codePage[ip];
        }
        void execute() {
            while (current().instruction != HALT) {
                nextInstruction();
                switch(current().instruction) {
                    case LAB: {

                    } break;
                    case JMP: {
                            int next = findLabel(getString(current().operand), LAB);
                            //cout<<"jump to label at: "<<next<<endl;
                            ip = next;
                    } break;
                    case JPC: {
                        if (getBoolean(stack[sp]) == false) {
                            int next = findLabel(getString(current().operand), LAB);
                            //cout<<"jump to label at: "<<next<<endl;
                            ip = next;
                        }
                        sp--;
                        stack[sp+1] = makeInt(0);
                    } break;
                    case LDC: {
                        sp += 1;
                        stack[sp] = current().operand;
                    } break;
                    case LOD: {
                        sp += 1;
                        int addr = calculateAddress(getInteger(current().operand));
                        stack[sp] = stack[addr];
                    } break;
                    case LDA: {
                        sp += 1;
                        int addr = calculateAddress(getInteger(current().operand));
                        stack[sp] = makeInt(addr);
                    } break;
                    case LDP: {
                        sp += 1;
                        int os = getInteger(current().operand);
                        int addr = os < 2000 ? getInteger(stack[bp+1])+4 + os:os;
                        stack[sp] = stack[addr];
                    } break;
                    case STO: {
                        int addr = calculateAddress(getInteger(stack[sp-1]));
                        stack[addr] = stack[sp];
                        sp -= 2;
                    } break;
                    case STP: {
                        int addr = calculateAddress(getInteger(stack[sp]));
                        stack[addr] = stack[sp-1];
                        sp -= 2;
                    } break;
                    case STN: {
                        int addr = calculateAddress(getInteger(stack[sp-1]));
                        stack[addr] = stack[sp];
                        stack[sp-1] = stack[sp];
                        sp -= 1;
                    } break;
                    case MST: {
                        stack[sp+1] = makeInt(bp);
                        stack[sp+2] = makeInt(bp);
                        stack[sp+3] = makeInt(ip);
                        bp = sp+1;
                        sp += 4;
                    } break;
                    case CAL: {
                        stack[bp+2] = makeInt(ip);
                        ip = findLabel(getString(current().operand), ENT);
                    } break;
                    case ENT: {

                    } break;
                    case RET: {
                        stack[bp] = stack[sp];
                        sp = bp;
                        ip = getInteger(stack[bp+2]);
                        bp = getInteger(stack[bp+1]);
                        lv--;
                    } break;
                    case ADD: {
                        sp -= 1;
                        stack[sp] = add(stack[sp], stack[sp+1]);
                        stack[sp+1] = makeInt(0);
                    } break;
                    case SUB: {
                        sp -= 1;
                        stack[sp] = sub(stack[sp], stack[sp+1]);
                        stack[sp+1] = makeInt(0);
                    } break;
                    case MUL: {
                        sp -= 1;
                        stack[sp] = mul(stack[sp], stack[sp+1]);
                        stack[sp+1] = makeInt(0);
                    } break;
                    case DIV: {
                        sp -= 1;
                        stack[sp] = div(stack[sp], stack[sp+1]);
                        stack[sp+1] = makeInt(0);
                    } break;
                    case EQU:{
                        sp -= 1;
                        stack[sp] = equ(stack[sp], stack[sp+1]);
                        stack[sp+1] = makeInt(0);
                    } break;
                    case NEQ: {
                        sp -= 1;
                        stack[sp] = neq(stack[sp], stack[sp+1]);
                        stack[sp+1] = makeInt(0);
                    } break;
                    case LTE: {
                        sp -= 1;
                        stack[sp] = lte(stack[sp], stack[sp+1]);
                        stack[sp+1] = makeInt(0);
                    } break;
                    case GTE: {
                        sp -= 1;
                        stack[sp] = gte(stack[sp], stack[sp+1]);
                        stack[sp+1] = makeInt(0);
                    } break;
                    case LT: {
                        sp -= 1;
                        stack[sp] = lt(stack[sp], stack[sp+1]);
                        stack[sp+1] = makeInt(0);
                    } break;
                    case GT: {
                        sp -= 1;
                        stack[sp] = gt(stack[sp], stack[sp+1]);
                        stack[sp+1] = makeInt(0);
                    } break;
                    case NEG: {
                        stack[sp] = neg(stack[sp]);
                    } break;
                    case PRINT: {
                        cout<<"\t\t\t\t\t"<<*toString(stack[sp])<<endl;
                        sp--;
                    } break;
                    case INC: {
                        sp +=1;
                        stack[sp] = makeInt(0);
                    } break;
                    case TS: {
                        sp += 1;
                        stack[sp] = makeInt(sp);
                    } break;
                    case HALT:
                    default:    break;
                }
                if (should_trace)
                    printStack();
            }
        }
        void printStack() {
            int arn = 0;
            cout<<"BP: "<<bp<<", SP: "<<sp<<" "<<endl;
            cout<<"{ ";
            for (int i = 1; i <= sp; i++) {
                if (i == bp) cout<<"\n(BP ";
                if (i == sp) cout<<"SP";
                cout<<"["<<i<<": "<<*toString(stack[i])<<"] ";
                if (i == sp) cout<<") ";
            }
            cout<<"}"<<endl;
            cout<<"------------"<<endl;
        }
};

#endif

//let x := 1; func count() { if (x <= 5) { println x; x := x + 1; count(); } else { println "fin."; } }; count();
// func fact(let k) { if (k < 2) { return 1; } else { return k*fact(k-1); } }; println fact(4);