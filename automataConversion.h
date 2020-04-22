#ifndef AUTOMATACONVERSION_H_INCLUDED
#define AUTOMATACONVERSION_H_INCLUDED
#include<iostream>
#include<vector>
#include<set>
#define MAX_STATES 1000
using namespace std;

class automata {
private:
    int nrStates;
    int startState;
    int alphabetSize;
    int nrOfFinalStates;
    int nrEdges;
    int nrDeadEnd;
    int inaccessible;
    set<char>alphabet;
    vector<pair<int, char>>g[MAX_STATES];
    bool isFinal[MAX_STATES];
public:
    automata();
    ~automata();
    void readAutomata(string);
    void printAutomata(string);
    void DFS(int, vector<bool>&, vector<int>&);
    void DFS(int, vector<bool>&);
    void statesWithCurrentCh(int, char , set<int>&);
    void updateStates(int, int );
    int getTrans(int, char);
    automata lambdaNFAtoNFA();
    automata NFAtoDFA();
    automata DFAtoDFAmin();
};

#endif // AUTOMATACONVERSION_H_INCLUDED
