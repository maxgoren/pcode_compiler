#ifndef stringbuffer_hpp
#define stringbuffer_hpp
#include <iostream>
using namespace std;

class StringBuffer {
    private:
        string str;
        int spos;
    public:
        StringBuffer() {

        }
        void init(string curr) {
            spos = 0;
            str = curr;
        }
        void advance() {
            spos++;
        }
        bool done() {
            return spos == str.size();
        }
        char get() {
            return str[spos];
        }
        void rewind() {
            if (spos-1 >= 0)
                spos--;
        }
        string data() {
            return str;
        }
};

#endif