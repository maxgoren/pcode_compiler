#ifndef lexer_hpp
#define lexer_hpp
#include <vector>
#include "token.hpp"
#include "tokenstream.hpp"
#include "stringbuffer.hpp"
using namespace std;

class Lexer {
    private:
        void skipWhiteSpace(StringBuffer& sb) {
            while (!sb.done()) {
                if (sb.get() == ' ' || sb.get() == '\t' || sb.get() == '\r') {
                    sb.advance();
                } else break;
            }
        }
        void skipComments(StringBuffer& sb) {
            if (sb.get() == '#') {
                sb.nextLine();
                return;
            }
            if (sb.get() == '{') {
                sb.advance();
                if (sb.get() == '*') {
                    sb.advance();
                    while (!sb.done()) {
                        if (sb.get() == '*') {
                            sb.advance();
                            if (sb.get() == '}') {
                                sb.advance();
                                skipWhiteSpace(sb);
                                return;
                            }
                        }
                        sb.advance();
                    }
                }
                sb.rewind();
            }
        }
        Token extractNumber(StringBuffer& sb) {
            string num;
            while (!sb.done()) {
                if (isdigit(sb.get()) || sb.get() == '.') {
                    num.push_back(sb.get());
                    sb.advance();
                } else break;
            }
            return Token(TK_NUM, num);
        }
        Token extractId(StringBuffer& sb) {
            string id;
            while (!sb.done()) {
                if (isalpha(sb.get()) || isdigit(sb.get())) {
                    id.push_back(sb.get());
                    sb.advance();
                } else break;
            }
            return checkReserved(id);
        }
        Token extractString(StringBuffer& sb) {
            string str;
            sb.advance();
            while (!sb.done()) {
                if (sb.get() == '"') 
                    break;
                str.push_back(sb.get());
                sb.advance();
            }
            if (sb.get() == '"') {
                sb.advance();
            } else {
                cout<<"Error: unterminated string."<<endl;
            }
            return Token(TK_STR, str);
        }
        Token checkReserved(string id) {
            if (id == "if")      return Token(TK_IF, "if");
            if (id == "do")      return Token(TK_DO, "do");
            if (id == "else")    return Token(TK_ELSE, "else");
            if (id == "then")    return Token(TK_THEN, "then");
            if (id == "let")     return Token(TK_LET, "let");
            if (id == "var")     return Token(TK_LET, "var");
            if (id == "println") return Token(TK_PRINT, "println");
            if (id == "return")  return Token(TK_RETURN, "return");
            if (id == "while")   return Token(TK_WHILE, "while");
            if (id == "program") return Token(TK_PROGRAM, "program");
            if (id == "procedure") return Token(TK_FUNC, "procedure");
            if (id == "def")     return Token(TK_FUNC, "def");
            if (id == "struct")  return Token(TK_STRUCT, "struct");
            if (id == "record")  return Token(TK_STRUCT, "record");
            if (id == "begin")   return Token(TK_BEGIN, "begin");
            if (id == "end")     return Token(TK_END, "end");
            if (id == "new")     return Token(TK_NEW, "new");
            if (id == "ref")     return Token(TK_REF, "ref");
            if (id == "matchre")   return Token(TK_MATCH, "matchre");
            return Token(TK_ID, id);
        }
        Token checkSpecials(StringBuffer& sb) {
            switch (sb.get()) {
                case '*': return Token(TK_MUL, "*");
                case '/': return Token(TK_DIV, "/");
                case '(': return Token(TK_LP, "(");
                case ')': return Token(TK_RP, ")");
                case '[': return Token(TK_LB, "[");
                case ']': return Token(TK_RB, "]");
                case '{': return Token(TK_BEGIN, "{");
                case '}': return Token(TK_END, "}");
                case '&': return Token(TK_FUNC, "&");
                case '.': return Token(TK_PERIOD, ".");
                case ',': return Token(TK_COMA, ",");
                case ';': return Token(TK_SEMI, ";");
                case '!': return Token(TK_NOT, "!");
                default: break;
            }
            if (sb.get() == '+') {
                sb.advance();
                if (sb.get() == '+') {
                    return Token(TK_POST_INC, "++");
                }
                sb.rewind();
                return Token(TK_ADD, "+");
            }
            if (sb.get() == '-') {
                sb.advance(); 
                if (sb.get() == '>') {
                    return Token(TK_PRODUCES, "->");
                } else if (sb.get() == '-') {
                    return Token(TK_POST_DEC, "--");
                }
                sb.rewind();
                return Token(TK_SUB, "-");
            }
            if (sb.get() == '<') {
                sb.advance();
                if (sb.get() == '=') {
                    return Token(TK_LTE, "<=");
                }
                sb.rewind();
                return Token(TK_LT, "<");
            }
            if (sb.get() == '>') {
                sb.advance();
                if (sb.get() == '=') {
                    return Token(TK_GTE, ">=");
                }
                sb.rewind();
                return Token(TK_GT, ">");
            }
            if (sb.get() == '=') {
                sb.advance();
                if (sb.get() == '=') {
                    return Token(TK_EQU, "==");
                }
                sb.rewind();
            }
            if (sb.get() == '!') {
                sb.advance();
                if (sb.get() == '=') {
                    return Token(TK_NEQ, "!=");
                }
                sb.rewind();
            }
            if (sb.get() == ':') {
                sb.advance();
                if (sb.get() == '=') {
                    return Token(TK_ASSIGN, ":=");
                }
                sb.rewind();
                return Token(TK_COLON, ":");
            }
            return Token(TK_ERR, "err");
        }
    public:
        Lexer() {

        }
        TokenStream lex(StringBuffer sb) {
            vector<Token> tokens;
            while (!sb.done()) {
                skipWhiteSpace(sb);
                skipComments(sb);
                if (isdigit(sb.get())) {
                    tokens.push_back(extractNumber(sb));
                } else if (isalpha(sb.get())) {
                    tokens.push_back(extractId(sb));
                } else if (sb.get() == '"') {
                    tokens.push_back(extractString(sb));
                } else {
                    tokens.push_back(checkSpecials(sb));
                    sb.advance();
                }
                if (tokens.back().symbol == TK_ERR) {
                    sb.rewind();
                    cout<<"Error on line: "<<sb.lineNo()<<", unknown token: "<<sb.get()<<endl;
                    tokens.clear();
                    break;
                }
            }
            tokens.push_back(Token(TK_EOI, "<fin.>"));
            return TokenStream(tokens);
        }
};

#endif