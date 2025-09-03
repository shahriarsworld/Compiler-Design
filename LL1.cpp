#include <bits/stdc++.h>
using namespace std;

map<char, vector<string>> grammar;   // Grammar rules
map<char, set<char>> FIRST, FOLLOW;  // FIRST and FOLLOW sets
map<char, map<char, string>> parsingTable;
set<char> terminals, nonTerminals;
char startSymbol;

// Function to check if symbol is terminal
bool isTerminal(char c) {
    return !(c >= 'A' && c <= 'Z');
}

// Compute FIRST(X)
set<char> computeFIRST(string str) {
    set<char> result;
    if (str.empty()) {
        result.insert('ε'); // epsilon
        return result;
    }

    char firstChar = str[0];
    if (isTerminal(firstChar)) {
        result.insert(firstChar);
        return result;
    }

    for (auto prod : grammar[firstChar]) {
        set<char> firstSet = computeFIRST(prod);
        result.insert(firstSet.begin(), firstSet.end());
    }

    if (result.count('ε')) {
        result.erase('ε');
        set<char> rest = computeFIRST(str.substr(1));
        result.insert(rest.begin(), rest.end());
    }

    return result;
}

// Compute FOLLOW(A)
void computeFOLLOW(char A) {
    if (FOLLOW.count(A)) return;

    if (A == startSymbol) FOLLOW[A].insert('$');

    for (auto &g : grammar) {
        char lhs = g.first;
        for (string rhs : g.second) {
            for (int i = 0; i < rhs.size(); i++) {
                if (rhs[i] == A) {
                    if (i + 1 < rhs.size()) {
                        set<char> firstNext = computeFIRST(rhs.substr(i + 1));
                        for (char f : firstNext) {
                            if (f != 'ε') FOLLOW[A].insert(f);
                        }
                        if (firstNext.count('ε')) {
                            computeFOLLOW(lhs);
                            FOLLOW[A].insert(FOLLOW[lhs].begin(), FOLLOW[lhs].end());
                        }
                    } else {
                        if (lhs != A) {
                            computeFOLLOW(lhs);
                            FOLLOW[A].insert(FOLLOW[lhs].begin(), FOLLOW[lhs].end());
                        }
                    }
                }
            }
        }
    }
}

// Build LL(1) Parsing Table
void buildParsingTable() {
    for (auto &g : grammar) {
        char A = g.first;
        for (string prod : g.second) {
            set<char> firstSet = computeFIRST(prod);

            for (char t : firstSet) {
                if (t != 'ε') parsingTable[A][t] = prod;
            }

            if (firstSet.count('ε')) {
                for (char b : FOLLOW[A]) {
                    parsingTable[A][b] = "ε";
                }
            }
        }
    }
}

// Print LL(1) Parsing Table
void printParsingTable() {
    cout << "\nLL(1) Parsing Table:\n";
    cout << setw(8) << " ";
    for (char t : terminals) cout << setw(8) << t;
    cout << setw(8) << "$";
    cout << "\n";

    for (char nt : nonTerminals) {
        cout << setw(8) << nt;
        for (char t : terminals) {
            if (parsingTable[nt].count(t))
                cout << setw(8) << parsingTable[nt][t];
            else
                cout << setw(8) << " ";
        }
        if (parsingTable[nt].count('$'))
            cout << setw(8) << parsingTable[nt]['$'];
        else
            cout << setw(8) << " ";
        cout << "\n";
    }
}

int main() {
    // Grammar:
    // E → TA
    // A → +TA | ε
    // T → FB
    // B → *FB | ε
    // F → (E) | i

    grammar['E'] = {"TA"};
    grammar['A'] = {"+TA", "ε"};
    grammar['T'] = {"FB"};
    grammar['B'] = {"*FB", "ε"};
    grammar['F'] = {"(E)", "i"};

    startSymbol = 'E';

    nonTerminals = {'E','A','T','B','F'};
    terminals = {'i','+','*','(',')'};

    // Compute FOLLOW sets
    for (char nt : nonTerminals) {
        computeFOLLOW(nt);
    }

    buildParsingTable();
    printParsingTable();

    return 0;
}
