#ifndef astbuilder_hpp
#define astbuilder_hpp
#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"
#include "syntaxtree.hpp"
using namespace std;

class ASTBuilder {
    private:
        bool should_trace;
        Lexer lexer;
        Parser parser;
        void printTokens(TokenStream ts) {
            int i = 0;
            for (ts.start(); !ts.done(); ts.advance()) {
                cout<<i<<": ";
                printToken(ts.get());
                i++;
            }
        }
        void printAST(ASTNode* ast) {
            traverse(ast);
        }
    public:
        ASTBuilder(bool trace = false) {
            should_trace = trace;
        }
        void setTrace(bool trace) {
            should_trace = trace;
        }
        ASTNode* build(string str) {
            StringBuffer sb;
            sb.init(str);
            TokenStream ts = lexer.lex(sb);
            if (should_trace)
                printTokens(ts);
            ASTNode* ast = parser.parse(ts);
            if (should_trace)
                printAST(ast);
            return ast;
        }
        ASTNode* buildFromFile(string filename) {
            StringBuffer sb;
            sb.readFromFile(filename);
            TokenStream ts = lexer.lex(sb);
            if (should_trace)
                printTokens(ts);
            ASTNode* ast = parser.parse(ts);
            if (should_trace)
                printAST(ast);
            return ast;
        }
};

#endif