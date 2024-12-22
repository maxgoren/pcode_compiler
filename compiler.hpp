#ifndef compiler_hpp
#define compiler_hpp
#include <vector>
#include "astbuilder.hpp"
#include "codegen.hpp"
#include "vminst.hpp"
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

#endif