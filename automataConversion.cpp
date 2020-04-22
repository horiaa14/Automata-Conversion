#include"automataConversion.h"
#include<iostream>
#include<fstream>
#include<vector>
#include<set>
#include<map>
#include<queue>
#define MAX_STATES 1000
using namespace std;

automata::automata() {
    nrStates = 0;
    startState = -1;
    alphabetSize = nrOfFinalStates = nrEdges = 0;
    nrDeadEnd = 0;
    inaccessible = 0;
    for(int i = 0; i < MAX_STATES; ++i)
        isFinal[i] = false;
}

automata::~automata() {
    for(int i = 0; i < nrStates; ++i) {
        g[i].clear();
        g[i].shrink_to_fit();
    }
}

void automata::readAutomata(string nameFin) {
    int x, y;
    char ch;
    ifstream fin(nameFin);
    try {
        fin >> nrStates;
        if(nrStates < 0)
            throw("Numarul starilor nu poate fi negativ!");
        fin >> alphabetSize;
        if(alphabetSize <= 0)
            throw("Alfabetul trebuie sa contina caractere!");
        for(int i = 0; i < alphabetSize; ++i) {
            fin >> ch;
            alphabet.insert(ch);
        }
        fin >> startState;
        if(startState < 0 || startState >= nrStates)
            throw("Stare initiala invalida!");
        fin >> nrOfFinalStates;
        if(nrOfFinalStates < 0 || nrOfFinalStates > nrStates)
            throw("Numarul de stari finale este invalid!");
        for(int i = 0; i < nrOfFinalStates; ++i) {
            fin >> x;
            if(x < 0 || x >= nrStates)
                throw("Starea finala introdusa este gresita!");
            isFinal[x] = true;
        }
        fin >> nrEdges;
        if(nrEdges < 0)
            throw("Numarul de muchii nu poate fi negativ!");
    } catch(const char* error) {
        cout << "A aparut o eroare la citirea datelor!\n" << error << "\n";
    }

    for(int i = 0; i < nrEdges; ++i) {
        try {
            fin >> x >> ch >> y;
            if(x < 0 || x > nrStates || y < 0 || y > nrStates)
                throw("Nodurile introduse sunt gresite!");
            if(ch != '$' && alphabet.find(ch) == alphabet.end())
                throw("Caracterul introdus nu exista in alfabet!");
        } catch(const char* error) {
            cout << "A aparut o eroare la introducerea muchiei " << x << " " << ch << " " << y << " :";
            cout << error << "\n";
        }
        g[x].push_back({y, ch});
    }
    fin.close();
}

void automata::printAutomata(string nameFout) {
    ofstream fout(nameFout);
    fout << "Numarul de stari : " << nrStates - nrDeadEnd - inaccessible << "\n";
    fout << "Lungimea alfabetului : " << alphabet.size() << "\n";
    for(auto i : alphabet)
        fout << i << " ";
    fout << "\n";
    fout << "Starea initiala : " << startState << "\n";
    fout << "Numarul de stari finale : " << nrOfFinalStates << "\n";
    for(int i = 0; i < nrStates; ++i)
        if(isFinal[i])
            fout << i << " ";
    fout << "\n";
    fout << "Numarul de muchii : " << nrEdges << "\n";
    for(int i = 0; i < nrStates; ++i) {
        vector<pair<int, char>> ::iterator it;
        if(!g[i].empty()) {
            for(it = g[i].begin(); it != g[i].end(); ++it)
                fout << i << " " << (*it).second << " " << (*it).first << "\n";
        }
    }
    fout.close();
}

void automata::DFS(int node, vector<bool>& isVisited, vector<int>& p) {
    // functia parcurge graful in adancime mergand doar pe muchii etichetate cu lambda
    p.push_back(node);
    isVisited[node] = 1;
    vector<pair<int, char>> ::iterator it;
    for(it = g[node].begin(); it != g[node].end(); ++it) {
        if(isVisited[(*it).first] == 0 && (*it).second == '$')
            DFS((*it).first, isVisited, p);
    }
}

void automata::statesWithCurrentCh(int node, char ch, set<int>& s) {
    // Functia retine in multimea s toate nodurile adiacente cu node
    // muchia de la node la nodul vecin fiind etichetata cu caracterul ch
    vector<pair<int, char>> ::iterator it;
    for(it = g[node].begin(); it != g[node].end(); ++it)
        if((*it).second == ch)
            s.insert((*it).first);
}

void automata::updateStates(int x, int y) {
    // Functia inlocuieste y cu x peste tot in tabelul de tranzitii
    for(int i = 0; i < nrStates; ++i) {
        set<pair<int, char>> s;
        for(unsigned j = 0; j < g[i].size(); ++j)
            if(g[i][j].first != y) // daca nodul curent este diferit de y
                s.insert(make_pair(g[i][j].first, g[i][j].second)); // copiez tranzitia pur si simplu
            else
                s.insert(make_pair(x, g[i][j].second)); // altfel inlocuiesc y cu x
        g[i].clear(); // sterg lista de adiacenta a lui i pentru a o reconstrui tinand cont de substitutie
        set<pair<int, char>> ::iterator s_it;
        for(s_it = s.begin(); s_it != s.end(); ++s_it) {
            g[i].push_back(make_pair((*s_it).first, (*s_it).second)); // actualizez lista de adiacenta a nodului i
        }
    }
}

automata automata::lambdaNFAtoNFA() {
    // Functia realizeaza conversia de la un l-NFA la un NFA
    automata nfa;
    vector<vector<int>> lambdaClosure(this->nrStates); // matricea care retine lambda-inchiderea
    vector<bool> isVisited(this->nrStates); // vectorul utilizat in parcurgerea DFS
    set<char>::iterator s_it;
    set<int>::iterator s_it2;
    bool ok;
    nfa.nrStates = this->nrStates;
    nfa.startState = this->startState;
    nfa.alphabetSize = this->alphabetSize;
    nfa.alphabet = this->alphabet;
    // Retinem in matricea lambdaClosure lambda-inchiderea pentru fiecare nod
    for(int i = 0; i < nrStates; ++i) {
        for(int j = 0; j < nrStates; ++j)
            isVisited[j] = 0;
        DFS(i, isVisited, lambdaClosure[i]);
    }
    // In acest moment avem calculata lambda inchiderea pentru fiecare nod
    for(s_it = alphabet.begin(); s_it != alphabet.end(); ++s_it) {
        for(int j = 0; j < nrStates; ++j) {
            // In s retinem starile in care putem ajunge cu caracterul curent (*s_it)
            // plecand din nodurile care fac parte din lambda inchiderea starii curente (j)
            set<int> s;
            for(unsigned k = 0; k < lambdaClosure[j].size(); ++k)
                statesWithCurrentCh(lambdaClosure[j][k], (*s_it), s);
            set<int> a;
            // In a vom retine lambda inchiderea multimii s
            // Multimea a va contine la final tranzitiile cu caracterul curent (*s_it)
            for(s_it2 = s.begin(); s_it2 != s.end(); ++s_it2) {
                for(unsigned k = 0; k < lambdaClosure[(*s_it2)].size(); ++k)
                    a.insert(lambdaClosure[(*s_it2)][k]);
            }
            // Adaugam la lista de adiacenta a nodului curent (j) tranzitia cu caracterul *s_it.
            for(s_it2 = a.begin(); s_it2 != a.end(); ++s_it2)
                nfa.g[j].push_back(make_pair((*s_it2), (*s_it)));
        }
    }
    // Starea initiala ramane aceeasi cu cea a automatului initial - a fost setata mai sus
    // Actualizam starile finale
    for(int i = 0; i < nrStates; ++i) {
        ok = false;
        // Daca lambda inchiderea nodului j contine o stare finala in automatul initial
        // Atunci starea curenta este stare finala
        for(unsigned j = 0; j < lambdaClosure[i].size() && !ok; ++j)
            if(this->isFinal[lambdaClosure[i][j]])
                ok = true;
        if(ok) {
            nfa.isFinal[i] = true;
            ++nfa.nrOfFinalStates;
        }
    }
    // Acum vom elimina starile redundante
    automata nfaC = nfa;
    for(int i = 0; i < nrStates; ++i) {
        for(int j = i + 1; j < nrStates; ++j) {
            // Daca starea i si starea j au tranzitiile identice pentru orice caracter din alfabet
            // Si ambele sunt sau nu stari finale, atunci vom elimina starea j
            if(nfa.isFinal[i] == nfa.isFinal[j] && nfa.g[i] == nfa.g[j]) {
                nfaC.g[j].clear(); // sterg lista de adiacenta a nodului j
                if(nfaC.isFinal[j]) { // verificam daca starea j este finala
                    nfaC.nrOfFinalStates--;
                    nfaC.isFinal[j] = false;
                }
                nfaC.updateStates(i, j); // inlocuim j cu i peste tot in tabelul de tranzitii
            }
        }
    }
    // Calculam numarul de muchii
    for(int i = 0; i < nrStates; ++i)
        nfaC.nrEdges += nfaC.g[i].size();
    // returnam automatul construit (NFA)
    return nfaC;
}

automata automata::NFAtoDFA() {
    // Functia realizeaza conversia de la un NFA la un DFA
    automata dfa; // la final vom returna un DFA
    // tabelul de tranzitii care va retine si starile compuse
    // cheile vor fi de tip set<int>, deoarece starile pot fi compuse ( de tipul 01 )
    // valorile vor fi de tip vector de perechi, primul element al unui element din vector
    // fiind starea ( posibil compusa ) in care ne putem duce, iar cel de-al doilea element
    // este caracterul corespunzator muchiei respective
    map<set<int>, vector<pair<set<int>, char>>> mp;
    map<set<int>, bool> used; // pentru a sti daca o stare a mai fost introdusa in coada
    map<set<int>, int> nmap; // pentru a asocia unei stari compuse un numar natural
    queue<set<int>> q; // coada care retine starile curente
    set<int> currStates;
    int cnt, nrst;
    bool ok;
    //dfa.nrStates = this->nrStates; // se va modifica
    dfa.startState = this->startState; // starea initiala este aceeasi cu cea a automatului initial
    dfa.alphabetSize = this->alphabetSize;
    dfa.alphabet = this->alphabet;
    q.push({this->startState}); // adaug starea initiala a automatului in coada
    while(!q.empty()) {
        currStates = q.front(); // extrag starea din coada
        q.pop();
        used[currStates] = true; // o marchez ca fiind introdusa in coada
        vector<pair<set<int>, char>> v; // acest vector va retine la final intreaga linie
        // din tabelul de tranzitii pentru starea curenta ( currStates )
        for(auto ch : this->alphabet) { // pentru fiecare caracter din alfabet
            set<int> aux;
            for(auto currNode : currStates) { // parcurg starea curenta, care poate fi compusa
                for(auto adjNode : this->g[currNode]) // parcurg vecinii nodului curent
                    if(adjNode.second == ch) // daca muchia este etichetata cu caracterul ch
                        aux.insert(adjNode.first); // o adaug in multimea aux
            }
            // Acum am format tranzitia cu caracterul ch pentru starea curenta
            if(!aux.empty()) { // verificam ca multimea formata sa fie nevida
                v.push_back(make_pair(aux, ch)); // adaugam tranzitia cu caracterul ch in vectorul v
                if(!used[aux]) // daca starea formata nu a mai fost introdusa in coada
                    q.push(aux); // introducem noua stare in coada
            }
        }
        // la final adaugam vectorul construit (v) ( adica intreaga linie) in tabelul de tranzitii - in dreptul
        // starii curente (currStates)
        mp[currStates] = v;
    }

    cnt = nrStates;
    nrst = 0; // numarul starilor introduse in coada
    for(auto it : mp) {
        if(it.first.size() > 1)
            nmap[it.first] = cnt++; // daca starea curenta este compusa, ii asociem un numar natural
        else {
            if(isFinal[*(it.first.begin())]) { // starea curenta din coada este simpla
                dfa.isFinal[*(it.first.begin())] = true; // verific daca este stare finala in automatul initial
                dfa.nrOfFinalStates++; // actualizez numarul de stari finale pentru dfa
            }
        }
        nrst++;
    }
    dfa.nrStates = cnt;
    dfa.inaccessible = dfa.nrStates - nrst;
    int node, adjNode;
    map<set<int>, vector<pair<set<int>, char>>>::iterator map_it;
    set<int>::iterator set_it;
    vector<pair<set<int>, char>>::iterator v_it;
    // verificam care dintre starile compuse au in componenta o stare finala
    // a automatului initial
    for(auto it : nmap) { // parcurgem doar starile compuse
        ok = false;
        for(set_it = it.first.begin(); set_it != it.first.end() && !ok; ++set_it)
            if(this->isFinal[*set_it])
                ok = true;
        if(ok) {// actualizez starea corespunzatoare in DFA
            dfa.isFinal[it.second] = true;
            ++dfa.nrOfFinalStates;
        }
    }

    for(map_it = mp.begin(); map_it != mp.end(); ++map_it) {
        if((*map_it).first.size() > 1) // starea este compusa
            node = nmap[(*map_it).first]; // ii luam valoarea din tabela nmap
        else node = (*(*map_it).first.begin()); // starea e simpla
        // formam lista de adiacenta a nodului node
        for(v_it = (*map_it).second.begin(); v_it != (*map_it).second.end(); ++v_it) {
            if((*v_it).first.size() > 1) // stare compusa
                adjNode = nmap[(*v_it).first];
            else adjNode = (*(*v_it).first.begin()); // stare simpla ( formata dintr-un nod )
            dfa.g[node].push_back(make_pair(adjNode, (*v_it).second));
        }
    }
    // actualizam numarul muchiilor din DFA
    for(int i = 0; i < dfa.nrStates; ++i)
        dfa.nrEdges += dfa.g[i].size();
    // returnam DFA-ul construit
    return dfa;
}

int automata::getTrans(int node, char ch) {
    // Functia returneaza nodul adiacent lui node, nodul de start si node fiind unite
    // printr-o muchie etichetata cu caracterul ch
    // Daca nu exista un astfel de nod, returneaza -1
    for(auto i : g[node])
        if(i.second == ch)
            return i.first;
    return -1;
}

void automata::DFS(int node, vector<bool>& isVisited) {
    isVisited[node] = true;
    for(auto i : g[node])
        if(!isVisited[i.first])
            DFS(i.first, isVisited);
}

automata automata::DFAtoDFAmin() {
    // Functia realizeaza conversia de la un DFA la un DFA min
    automata dfaMin;
    set<set<int>>s;
    map<set<int>, vector<pair<set<int>, char>>> mp;
    map<set<int>, int> nmap;
    vector<bool> isVisited(nrStates);
    bool t[nrStates][nrStates];
    bool ok, deadEnd;
    int cnt, numberOfDeadEnd;
    int node1, node2, node, adjNode;
    char c;
    // Marcam matricea de adiacenta cu true
    for(int i = 0; i < nrStates; ++i)
        for(int j = 0; j < nrStates; ++j)
            t[i][j] = true;
    // Marcam cu false toate perechile (q, r) unde q este stare finala si r stare nefinala
    for(int i = 1; i < nrStates; ++i)
        for(int j = 0; j < i; ++j)
            if((isFinal[i] && !isFinal[j]) || (!isFinal[i] && isFinal[j]))
                t[i][j] = false;
    // Marcam cu false toate perechile (q, r) pentru care (delta(q, alpha), delta(r, alpha)) sunt
    // marcate cu false
    do { // repetam pana nu mai apar modificari
        ok = true;
        for(int i = 1; i < nrStates; ++i)
            for(int j = 0; j < i; ++j)
                if(t[i][j]) {
                    for(auto ch : alphabet) {
                        node1 = getTrans(i, ch);
                        node2 = getTrans(j, ch);
                        if(node1 < node2)
                            swap(node1, node2);
                        if(node1 >= 0 && node2 >= 0 && !t[node1][node2]) {
                            t[i][j] = false;
                            ok = false;
                            break;
                        }
                    }
                }

    }while(!ok);
    set<set<int>>::iterator s_it;
    set<int>::iterator it;
    // Grupam starile echivalente din matricea de echivalenta intr-o singura stare
    for(int i = 1; i < nrStates; ++i) {
        set<int>currSet;
        currSet.insert(i);
        for(int j = 0; j < i; ++j)
            if(t[i][j])
                currSet.insert(j);
        // Verificam daca multimea currSet este inclusa in alta multime ( din multimea s )
        for(s_it = s.begin(); s_it != s.end();) {
            ok = true;
            for(it = s_it->begin(); it != s_it->end() && ok; ++it)
                if(currSet.find(*it) == currSet.end())
                    ok = false;
            if(ok) {
                set<set<int>>::iterator aux;
                aux = s_it;
                ++s_it;
                s.erase(aux);
            } else ++s_it;
        }
        s.insert(currSet);
    }
    set<set<int>>::iterator set_it;
    for(set_it = s.begin(); set_it != s.end(); ++set_it) {
        vector<pair<set<int>, char>> v;
        for(auto ch : alphabet) {
            set<int> newSet;
            set<int>::iterator s2_it;
            for(s2_it = set_it->begin(); s2_it != set_it->end(); ++s2_it) {
                // pentru fiecare nod din starea compusa calculam tranzitiile cu caracterul ch
                node = getTrans(*s2_it, ch);
                if(node >= 0)
                    newSet.insert(node);
            }
            // verificam in ce multime este inclusa multimea newSet
            if(!newSet.empty()) {
                set<set<int>>::iterator set2_it;
                for(set2_it = s.begin(); set2_it != s.end(); ++set2_it) {
                    ok = false;
                    for(s2_it = set2_it->begin(); s2_it != set2_it->end() && !ok; ++s2_it) {
                        if(newSet.find(*s2_it) != newSet.end())
                            ok = true;
                    }
                    if(ok) {
                        v.push_back(make_pair(*set2_it, ch));
                        break;
                    }
                }
            }
        }
        if(!v.empty())
            mp[*set_it] = v;
    }

    // dupa ce am calculat functia de tranzitie, calculam starile initiale si finale
    set<set<int>>start; // starea initiala
    set<set<int>>fin; // starile finale
    bool isStartState;
    bool isFinalState;
    map<set<int>, vector<pair<set<int>, char>>>::iterator map_it;
    for(map_it = mp.begin(); map_it != mp.end(); ++map_it) {
        set<int>::iterator a;
        isStartState = false;
        isFinalState = true;
        for(a = (*map_it).first.begin(); a != (*map_it).first.end(); ++a) {
            if(*a == this->startState)
                isStartState = true;
            else if(!isFinal[*a])
                isFinalState = false;
        }
        if(isStartState)
            start.insert((*map_it).first);
        if(isFinalState)
            fin.insert((*map_it).first);
    }
    cnt = 0;
    for(map_it = mp.begin(); map_it != mp.end(); ++map_it)
        nmap[(*map_it).first] = cnt++;
    //cout << "Automatul are " << cnt << " stari!\n";
    // vom forma listele de adiacenta pentru automatul curent
    dfaMin.nrStates = cnt; // numarul de stari pentru automatul dfaMIN
    dfaMin.startState = nmap[*(start.begin())]; // actualizez starea initiala
    dfaMin.nrOfFinalStates = fin.size();
    for(s_it = fin.begin(); s_it != fin.end(); ++s_it)
        dfaMin.isFinal[nmap[*s_it]] = true;
    dfaMin.alphabetSize = alphabetSize;
    dfaMin.alphabet = alphabet;
    for(map_it = mp.begin(); map_it != mp.end(); ++map_it) { // parcurg starile compuse
        vector<pair<set<int>, char>>::iterator v_it;
        for(v_it = (*map_it).second.begin(); v_it != (*map_it).second.end(); ++v_it) {
            node = nmap[(*map_it).first]; // folosim tabela nmap pt a asocia starii compuse o singura stare
            adjNode = nmap[(*v_it).first];
            c = (*v_it).second;
            dfaMin.g[node].push_back(make_pair(adjNode, c));
            ++dfaMin.nrEdges;
        }
    }

    //cout << dfaMin.nrEdges;
    vector<bool> isDeadEnd(dfaMin.nrStates, false);
    numberOfDeadEnd = 0;
    for(int i = 0; i < dfaMin.nrStates; ++i) {
        for(int j = 0; j < dfaMin.nrStates; ++j)
            isVisited[j] = false;
        if(!dfaMin.isFinal[i] && i != dfaMin.startState) {
            dfaMin.DFS(i, isVisited);
            deadEnd = true;
            for(int j = 0; j < dfaMin.nrStates && deadEnd; ++j)
                if(isVisited[j] && dfaMin.isFinal[j])
                    deadEnd = false;

            if(deadEnd) {
                isDeadEnd[i] = true;
                ++numberOfDeadEnd;
            }
        }
    }
    //cout << "Numarul de stari dead-end este " << numberOfDeadEnd << "\n";
    // pentru a evita operatiile de stergere care sunt foarte costisitoare
    // vom construi un automat auxiliar din care vom elimina starile dead-end
    dfaMin.nrDeadEnd = numberOfDeadEnd;
    automata aux;
    aux.nrStates = dfaMin.nrStates;
    aux.startState = dfaMin.startState;
    aux.alphabetSize = dfaMin.alphabetSize;
    aux.nrOfFinalStates = dfaMin.nrOfFinalStates;
    aux.alphabet = dfaMin.alphabet;
    for(int i = 0; i < dfaMin.nrStates; ++i)
        aux.isFinal[i] = dfaMin.isFinal[i];
    for(int i = 0; i < dfaMin.nrStates; ++i) {
        if(!isDeadEnd[i]) { // nodul i nu este stare dead-end, deci ii construiesc lista de adiacenta
            vector<pair<int, char>>::iterator p;
            for(p = dfaMin.g[i].begin(); p != dfaMin.g[i].end(); ++p)
                if(!isDeadEnd[(*p).first]) { // nodul adiacent nu trebuie sa fie dead-end
                    aux.g[i].push_back(make_pair((*p).first, (*p).second));
                    ++aux.nrEdges;
                }
        }
    }

    // eliminam starile neaccesibile
    for(int i = 0; i < aux.nrStates; ++i)
        isVisited[i] = false;
    aux.DFS(aux.startState, isVisited); // facem o parcurgere DFS din nodul sursa
    // daca isVisited[x] == false, atunci x este inaccesibil din sursa, deci il vom elimina
    // folosim un automat auxiliar pentru a evita operatiile erase
    automata aux2;
    for(int i = 0; i < aux.nrStates; ++i) {
        vector<pair<int, char>>::iterator p;
        if(isVisited[i]) {// daca am ajuns in acest nod plecand din sursa, ii formez lista de adiacenta
            for(p = aux.g[i].begin(); p != aux.g[i].end(); ++p)
                if(isVisited[(*p).first]) {
                    aux2.g[i].push_back(make_pair((*p).first, (*p).second));
                    ++aux2.nrEdges;
            }
        } else if(!isDeadEnd[i]) { // daca e deadEnd, a fost sters la pasul anterior
            dfaMin.inaccessible++;
        }
    }

    dfaMin.nrEdges = aux2.nrEdges;
    for(int i = 0; i < dfaMin.nrStates; ++i)
        dfaMin.g[i] = aux2.g[i];
    return dfaMin;
}
