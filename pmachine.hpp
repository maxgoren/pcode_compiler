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
        const int MAX_STACK = 7000;
        const int HEAP_SIZE = 2000;
        const int MIN_GLOBAL_ADDR = MAX_STACK - HEAP_SIZE;
        const int MAX_GLOBAL_ADDR = 3000;
        vector<Instruction> codePage;
        vector<Value> stack;
        Instruction curr;
        int sp; //stack pointer
        int ip; //instruction pointer
        int bp; //base pointer
        int mp; //mark point;
        int sl; //dynamic link
        int dl; //static link
        int ra; //return addr
        Instruction current() {
            return curr;
        }
        int base(int lvl) {
            int np = bp;
            while (lvl > 0) {
                np = getValue(stack[np+1]);
                lvl--;
            }
            return np+4;
        }
        int getValue(Value val) {
            return  val.type == AS_INT ? getInteger(val):getReal(val);
        }
        int calculateAddress(int offset) {
            int bn = 0;
            if (offset < MAX_GLOBAL_ADDR)
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
        void doJump() {
            int next = findLabel(getString(current().operand), LAB);
            ip = next;
        }
        void jumpConditional() {
            if (getBoolean(stack[sp]) == false) {
                int next = findLabel(getString(current().operand), LAB);
                ip = next;
            }
            sp--;
            stack[sp+1] = makeInt(0);
        }
        void loadConstant() {
            sp += 1;
            stack[sp] = current().operand;
        }
        void loadFromAddress() {
            sp += 1;
            int addr = calculateAddress(getInteger(current().operand));
            stack[sp] = stack[addr];
        }
        void loadAddress() {
            sp += 1;
            int addr = calculateAddress(getInteger(current().operand));
            stack[sp] = makeInt(addr);
        }
        void loadParam() {
            sp += 1;
            int os = getValue(current().operand);
            int addr = os < 2000 ? getValue(stack[bp+1])+4 + os:os;
            stack[sp] = stack[addr];
        }
        void indirectLoad() {
            int indAddr = 0;
            int base = getValue(stack[sp]);
            int offset = getInteger(current().operand);
            if (base > MAX_GLOBAL_ADDR) {
                indAddr = base - offset;
            } else {
                indAddr = base + offset;
            }
            if (should_trace) {
                cout<<"Base Addr:  "<<base<<endl;
                cout<<"Offset:     "<<offset<<endl;
                cout<<"Indirected: "<<indAddr<<endl;
            }    
            stack[sp] = stack[indAddr];
        }
        void indexedAccess() {
            int tsval = getValue(stack[sp]);
            int scale = getInteger(current().operand); 
            int base = getValue(stack[sp-1]);
            int ixAddr = 0;
            if (base > MAX_GLOBAL_ADDR) {
                ixAddr = base - (tsval * scale);
            } else {
                ixAddr = base + (tsval * scale);
            }
            if (should_trace) {
                cout<<"Base Addr: "<<base<<endl;
                cout<<"Scaling: "<<scale<<endl;
                cout<<"offset : "<<tsval<<endl;
                cout<<"Indexed Address: "<<ixAddr<<endl;
            }
            sp -= 1;
            stack[sp] = makeInt(ixAddr);
        }
        void storeDestructive() {
            int addr = calculateAddress(getValue(stack[sp-1]));
            stack[addr] = stack[sp];
            sp -= 2;
        }
        void storeParam() {
            int addr = calculateAddress(getValue(stack[sp]));
            stack[addr] = stack[sp-1];
            sp -= 2;
        }
        void storeNonDestructive() {
            int addr = calculateAddress(getValue(stack[sp-1]));
            stack[addr] = stack[sp];
            stack[sp-1] = stack[sp];
            sp -= 1;
        }
        void markStack() {
            stack[sp+1] = makeInt(bp); dl = sp+1;
            stack[sp+2] = makeInt(bp); sl = sp+2;
            stack[sp+3] = makeInt(ip); ra = sp+3;
            bp = sp+1;
            sp += 4;
        }
        void callProcedure() {
            stack[bp+2] = makeInt(ip); ra = ip;
            ip = findLabel(getString(current().operand), ENT);
        }
        void returnFromProcedure() {
            stack[bp] = stack[sp];
            sp = bp;
            ip = getInteger(stack[bp+2]);
            bp = getInteger(stack[bp+1]); 
            dl = bp+1; 
            sl = bp+2;
            ra = bp+3;
        }
        void binaryOperator() {
            switch (current().instruction) {
                case ADD: {
                    sp -= 1;
                    stack[sp] = add(stack[sp], stack[sp+1]);
                } break;
                case SUB: {
                    sp -= 1;
                    stack[sp] = sub(stack[sp], stack[sp+1]);
                } break;
                case MUL: {
                    sp -= 1;
                    stack[sp] = mul(stack[sp], stack[sp+1]);
                } break;
                case DIV: {
                    sp -= 1;
                    stack[sp] = div(stack[sp], stack[sp+1]);
                } break;
                case EQU:{
                    sp -= 1;
                    stack[sp] = equ(stack[sp], stack[sp+1]);
                } break;
                case NEQ: {
                    sp -= 1;
                    stack[sp] = neq(stack[sp], stack[sp+1]);
                } break;
                case LTE: {
                    sp -= 1;
                    stack[sp] = lte(stack[sp], stack[sp+1]);
                } break;
                case GTE: {
                    sp -= 1;
                    stack[sp] = gte(stack[sp], stack[sp+1]);
                } break;
                case LT: {
                    sp -= 1;
                    stack[sp] = lt(stack[sp], stack[sp+1]);
                } break;
                case GT: {
                    sp -= 1;
                    stack[sp] = gt(stack[sp], stack[sp+1]);
                } break;
            };
        }
        inline void nop() { }
    public:
        PCodeVM(bool trace = false) {
            ip = 0;
            bp = 0;
            sp = 4;
            dl = 0;
            sl = 0;
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
                        nop();
                    } break;
                    case JMP: {
                        doJump();
                    } break;
                    case JPC: {
                        jumpConditional();
                    } break;
                    case LDC: {
                        loadConstant();
                    } break;
                    case LOD: {
                        loadFromAddress();
                    } break;
                    case LDA: {
                        loadAddress();
                    } break;
                    case LDP: {
                        loadParam();
                    } break;
                    case LDI: {
                        indirectLoad();
                    } break;
                    case IXA: {
                        indexedAccess();
                    } break;
                    case STO: {
                        storeDestructive();
                    } break;
                    case STP: {
                        storeParam();
                    } break;
                    case STN: {
                        storeNonDestructive();
                    } break;
                    case MST: {
                        markStack();
                    } break;
                    case CAL: {
                        callProcedure();
                    } break;
                    case ENT: {
                        nop();
                    } break;
                    case RET: {
                        returnFromProcedure();
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
                        break;
                    default:    
                        binaryOperator();
                        break;
                }
                if (should_trace)
                    printStack();
            }
        }
        void printStack() {
            int arn = 0;
            cout<<"[---------------------------------------------------]"<<endl;
            cout<<"Globals: ";
            for (int i = MIN_GLOBAL_ADDR; i > MIN_GLOBAL_ADDR - 20 && stack[i].type != AS_NIL; i--) 
                cout<<"["<<i<<": "<<*toString(stack[i])<<"] ";
            cout<<endl;
            cout<<"------------------------"<<endl;
            cout<<"Stack: BP: "<<bp<<", SP: "<<sp<<" "<<endl;
            cout<<"{ ";
            for (int i = 0; i <= sp; i++) {
                if (i == bp) cout<<"(BP: ";
                if (i == sp) cout<<"SP: ";
                if (sl != 0) {
                    if (i == sl) cout<<"SL: ";
                    if (i == dl) cout<<"DL: ";
                    if (i == ra) cout<<"RA: ";
                }
                cout<<"["<<i<<": "<<*toString(stack[i])<<"] ";
                if (i == sp) cout<<") ";
            }
            cout<<"}"<<endl;
            cout<<"[---------------------------------------------------]"<<endl;
        }
};

#endif

//let x := 1; func count() { if (x <= 5) { println x; x := x + 1; count(); } else { println "fin."; } }; count();
// func fact(let k) { if (k < 2) { return 1; } else { return k*fact(k-1); } }; let k := 1; while (k <= 8) { println fact(k); k := k + 1; }

// func fib(let k) { if (k < 2) { return 1; } else { return fib(k-1) + fib(k-2); } }; let fibs[15]; let i := 1; while (i < 15) { fibs[i] := fib(i); println fibs[i]; i := i + 1; }