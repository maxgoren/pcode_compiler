#include <iostream>
#include "compiler.hpp"
#include "pmachine.hpp"
using namespace std;

void repl(bool should_trace) {
    bool running = true;
    string buff;
    Compiler compiler;
    PCodeVM vm;
    while (running) {
        cout<<"repl> ";
        getline(cin, buff);
        if (buff == ".traceon") {
            vm.setTrace(true);
            compiler.setTrace(true);
            should_trace = true;
        } else if (buff == ".traceoff") {
            vm.setTrace(false);
            compiler.setTrace(false);
            should_trace = false;
        } else {
            auto pcode = compiler.compile(buff);
            if (should_trace) {
                for (auto p : pcode) {
                    cout<<p<<endl;
                    if (p.instruction == HALT)
                        break;
                }
            }
            vm.init(pcode);
            vm.execute();
        }
    }
}

void compileAndRunFromFile(string filename, bool trace) {
    Compiler compiler;
    PCodeVM vm;
    compiler.setTrace(trace);
    vm.setTrace(trace);
    auto pcode = compiler.compileFile(filename);
    int i = 0;
    for (auto p : pcode) {
        cout<<i++<<": "<<p<<endl;
        if (p.instruction == HALT)
            break;
    }
    vm.init(pcode);
    vm.execute();
}


int main(int argc, char* argv[]) {
    if (argc < 2)
        repl(false);
    switch (argc) {
        case 2:
            compileAndRunFromFile(argv[1], false);
            break;
        case 3:
            compileAndRunFromFile(argv[2], true);
        default:
            break;

    }
    return 0;
}