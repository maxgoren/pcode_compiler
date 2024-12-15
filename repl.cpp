#include <iostream>
#include "astbuilder.hpp"
#include "codegen.hpp"
#include "pmachine.hpp"
using namespace std;

void repl() {
    bool running = true;
    string buff;
    ASTBuilder astBuilder;
    PCodeGenerator gen;
    PCodeVM vm;
    while (running) {
        cout<<"repl> ";
        getline(cin, buff);
        if (buff == ".traceon") {
            vm.setTrace(true);
        } else if (buff == ".traceoff") {
            vm.setTrace(false);
        } else {
            auto syntaxTree = astBuilder.build(buff);
            auto pcode = gen.generate(syntaxTree);
            for (auto p : pcode) {
                cout<<p<<endl;
            }
            vm.init(pcode);
            vm.execute();
        }
    }
}


int main(int argc, char* argv[]) {
    repl();
}