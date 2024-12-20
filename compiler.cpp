#include <iostream>
#include "astbuilder.hpp"
#include "codegen.hpp"
#include "pmachine.hpp"
using namespace std;

class Compiler {
    private:
        ASTBuilder astBuilder;
        PCodeGenerator codeGenerator;
    public:
        Compiler(bool trace = false) {
            astBuilder.setTrace(trace);
            codeGenerator.setTrace(trace);
        }
        vector<Instruction> compile(string code) {
            ASTNode* ast = astBuilder.build(code);
            return codeGenerator.generate(ast);
        }
        vector<Instruction> compileFile(string filename) {
            ASTNode* ast = astBuilder.buildFromFile(filename);
            return codeGenerator.generate(ast);
        }
        void setTrace(bool trace) {
            astBuilder.setTrace(trace);
            codeGenerator.setTrace(trace);
        }
};

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
        } else if (buff == ".traceoff") {
            vm.setTrace(false);
            compiler.setTrace(false);
        } else {
            auto pcode = compiler.compile(buff);
            if (should_trace) {
                for (auto p : pcode) {
                    cout<<p<<endl;
                }
            }
            vm.init(pcode);
            vm.execute();
        }
    }
}

void runFile(string filename, bool trace) {
    Compiler compiler;
    PCodeVM vm;
    compiler.setTrace(trace);
    vm.setTrace(trace);
    auto pcode = compiler.compileFile(filename);
    vm.init(pcode);
    vm.execute();
}


int main(int argc, char* argv[]) {
    if (argc < 2)
        repl(false);
    switch (argc) {
        case 2:
            runFile(argv[1], false);
            break;
        case 3:
            runFile(argv[2], true);
        default:
            break;

    }
    return 0;
}