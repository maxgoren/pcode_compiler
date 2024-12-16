#ifndef astbuilder_hpp
#define astbuilder_hpp
#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"
#include "syntaxtree.hpp"
using namespace std;

class ASTBuilder {
    private:
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
        ASTBuilder() {

        }
        ASTNode* build(string str) {
            StringBuffer sb;
            sb.init(str);
            TokenStream ts = lexer.lex(sb);
            printTokens(ts);
            ASTNode* ast = parser.parse(ts);
            printAST(ast);
            return ast;
        }
};

#endif