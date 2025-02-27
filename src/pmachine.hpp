#ifndef pmachine_hpp
#define pmachine_hpp
#include <iomanip>
#include <iostream>
#include <vector>
#include "regex/re_compiler.hpp"
#include "regex/patternmatcher.hpp"
#include "regex/nfa.hpp"
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
                cout<<"Calculated ad: "<<bn+offset<<endl;
            }
            return bn+offset;
        }
        void nextInstruction() {
            curr = codePage[ip++];
            if (should_trace)
                cout<<"Executing: "<<ip-1<<": "<<instStr[current().instruction]<<" "<<*toString(current().operand)<<" "<<*toString(current().nestlevel)<<endl;
        }
        void doJump() {
            int next = getInteger(current().operand);
            ip = next;
        }
        void jumpConditional() {
            if (getBoolean(stack[sp]) == false) {
                int next = getInteger(current().operand);
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
        void loadReferenceParam() {
            sp += 1;
            int os = getValue(current().operand);
            int addr = os < 2000 ? getValue(stack[bp+1])+4 + os:os;
            stack[sp] = makeInt(addr);
        }
        void loadParam() {
            sp += 1;
            int os = getValue(current().operand);
            int addr = os < 2000 ? getValue(stack[bp+1])+4 + os:os;
            stack[sp] = stack[addr];
        }
        void loadField() {
            sp += 1;
            int os = getValue(current().operand);
            int addr = os < 2000 ? getValue(stack[bp+1])+1 + os:os;
            stack[sp] = makeInt(addr);
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
                cout<<"Base Addr:  "<<base<<", Offset:     "<<offset<<endl;
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
                cout<<"Base Addr: "<<base<<", Scaling: "<<scale<<", offset : "<<tsval<<endl;
                cout<<"Indexed Address: "<<ixAddr<<endl;
            }
            sp -= 1;
            stack[sp] = makeInt(ixAddr);
        }
        void storeDestructive() {
            int addr = getValue(stack[sp-1]);
            if (should_trace) {
                cout<<"Calculated ad: "<<addr<<endl;
            }
            stack[addr] = stack[sp];
            sp -= 2;
        }
        void matchRegExp() {
            string pattern = string(getString(stack[sp])->str);
            sp--;
            string text = string(getString(stack[sp])->str);
            sp--;
            cout<<"text: "<<text<<endl;
            cout<<"Pattern: "<<pattern<<endl;
            NFACompiler reCompiler;
            NFA nfa = reCompiler.compile(pattern);
            RegExPatternMatcher pm(nfa, should_trace);
            stack[++sp] = makeBool(pm.match(text));
        }
        void storeParam() {
            int addr = calculateAddress(getValue(stack[sp]));
            stack[addr] = stack[sp-1];
            sp -= 2;
        }
        void storeNonDestructive() {
            int addr = getValue(stack[sp-1]);
            if (should_trace) {
                cout<<"Calculated ad: "<<addr<<endl;
            }
            stack[addr] = stack[sp];
            stack[sp-1] = stack[sp];
            sp -= 1;
        }
        void markStack() {
            stack[sp+1] = makeInt(bp); dl = sp+1;  //dynamic link
            stack[sp+2] = makeInt(bp); sl = sp+2;  //static link
            stack[sp+3] = makeInt(ip); ra = sp+3;  //return address
            bp = sp+1;                             //set new base ptr
            sp += 4;                               //advance stack ptr
        }
        void callProcedure() {
            stack[bp+2] = makeInt(ip); ra = bp+2;   //update return address
            ip = getInteger(current().operand);     //set instruction ptr
        }
        void returnFromProcedure() {
            stack[bp] = stack[sp];          //put return value at space saved for it
            sp = bp;                        //reset stack ptr
            ip = getInteger(stack[bp+2]);   //reset instruction ptr;
            bp = getInteger(stack[bp+1]);   //reset base ptr
            dl = bp;                        //dynamic link
            sl = bp+1;                      //static link
            ra = bp+2;                      //return address
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
            dl = 1;
            sl = 2;
            ra = 3;
            stack.reserve(MAX_STACK);
            codePage.reserve(MAX_STACK);
            should_trace = trace;
        }
        void setTrace(bool trace) {
            should_trace = trace;
        }
        void init(vector<Instruction>& code) {
            for (int i = 0; i < code.size(); i++)
                codePage[i] = code[i];
            if (ip > 0) ip--;
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
                    case LRP: {
                        loadReferenceParam();
                    } break;
                    case LDP: {
                        loadParam();
                    } break;
                    case LDF: {
                        loadField();
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
                    case NOT: {
                        stack[sp] = Not(stack[sp]);
                    } break;
                    case PRINT: {
                        cout<<"\t\t\t\t\t"<<*toString(stack[sp])<<endl;
                        sp--;
                    } break;
                    case MATCHRE: {
                        matchRegExp();
                    } break;
                    case INC: {
                        for (int i = 0; i < getInteger(current().operand); i++) {
                            sp += 1;
                            stack[sp] = makeInt(0);
                        }
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
                //if (should_trace && current().instruction != HALT)
                //    printStack();
            }
        }
        void printStack() {
            int arn = 0;
            cout<<"[---------------------------------------------------]"<<endl;
            cout<<"{ \n";
            cout<<"  BP: "<<bp<<", SP: "<<sp<<endl;
            cout<<"      Stack:         \t\t\tGlobals: "<<endl;
            for (int i = 0, j = 5000; i <= sp; i++, j--) {
                if (i == bp) cout<<" BP: ";
                if (i == sp) cout<<" SP: ";
                if (i == sl) cout<<" SL: ";
                if (i == dl && dl != bp) cout<<" DL: ";
                if (i == ra && i != sp) cout<<" RA: ";
                if (i != bp && i != sp && i != sl && i != dl && i != ra) 
                    cout<<"     ";
                cout<<"["<<setw(4)<<i<<": "<<setw(15)<<*toString(stack[i])<<"] ";
                cout<<"\t\t";
                cout<<"["<<setw(4)<<j<<": "<<setw(15)<<*toString(stack[j])<<"] "<<endl;

            }
            cout<<"}"<<endl;
            cout<<"[---------------------------------------------------]"<<endl;
        }
};

#endif