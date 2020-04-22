#include<iostream>
#include"automataConversion.h"
using namespace std;

int main() {
    /*// -- l-NFA to NFA
    automata x, y, z;
    x.readAutomata("lambdaNFA.in");
    x = x.lambdaNFAtoNFA();
    x.printAutomata("NFA.out");
    // NFA to DFA
    y.readAutomata("NFA.in");
    y = y.NFAtoDFA();
    y.printAutomata("DFA.out");
    // DFA to DFAmin
    z.readAutomata("DFA.in");
    z = z.DFAtoDFAmin();
    z.printAutomata("DFAmin.out");*/
    automata a;
    a.readAutomata("lambdaNFA.in");
    a = a.lambdaNFAtoNFA();
    a.printAutomata("NFA.out");
    a = a.NFAtoDFA();
    a.printAutomata("DFA.out");
    a = a.DFAtoDFAmin();
    a.printAutomata("DFAmin.out");
    return 0;
}
