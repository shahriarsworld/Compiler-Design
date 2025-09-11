#include <bits/stdc++.h>
using namespace std;

map<string, vector<vector<string>>> grammar;      // Grammar productions
map<string, set<string>> firstSets, followSets;  // FIRST and FOLLOW sets
map<string, map<string, vector<string>>> table;  // Parsing table
set<string> terminals, nonTerminals;
string startSymbol;

// Utility: check if a symbol is terminal
bool isTerminal(const string &sym) {
    return !(sym[0] >= 'A' && sym[0] <= 'Z');
}

// Compute FIRST set
set<string> computeFirst(const string &sym) {
    if (isTerminal(sym)) return {sym};
    if (!firstSets[sym].empty()) return firstSets[sym];

    for (auto prod : grammar[sym]) {
        bool epsilonFound = true;
        for (auto token : prod) {
            set<string> temp = computeFirst(token);
            for (auto x : temp) if (x != "ε") firstSets[sym].insert(x);
            if (temp.find("ε") == temp.end()) {
                epsilonFound = false;
                break;
            }
        }
        if (epsilonFound) firstSets[sym].insert("ε");
    }
    return firstSets[sym];
}

// Compute FOLLOW set
set<string> computeFollow(const string &sym) {
    if (!followSets[sym].empty()) return followSets[sym];

    if (sym == startSymbol) followSets[sym].insert("$"); // End marker

    for (auto &[lhs, prods] : grammar) {
        for (auto prod : prods) {
            for (int i = 0; i < prod.size(); i++) {
                if (prod[i] == sym) {
                    if (i + 1 < prod.size()) {
                        auto nextFirst = computeFirst(prod[i + 1]);
                        for (auto x : nextFirst)
                            if (x != "ε") followSets[sym].insert(x);

                        if (nextFirst.find("ε") != nextFirst.end()) {
                            auto temp = computeFollow(lhs);
                            followSets[sym].insert(temp.begin(), temp.end());
                        }
                    } else {
                        if (lhs != sym) {
                            auto temp = computeFollow(lhs);
                            followSets[sym].insert(temp.begin(), temp.end());
                        }
                    }
                }
            }
        }
    }
    return followSets[sym];
}

// Build Parsing Table
void constructParsingTable() {
    for (auto &[lhs, prods] : grammar) {
        for (auto prod : prods) {
            set<string> firstSet;
            bool epsilonFound = true;
            for (auto token : prod) {
                auto temp = computeFirst(token);
                firstSet.insert(temp.begin(), temp.end());
                if (temp.find("ε") == temp.end()) {
                    epsilonFound = false;
                    break;
                }
            }

            for (auto x : firstSet) {
                if (x != "ε")
                    table[lhs][x] = prod;
            }

            if (epsilonFound || firstSet.find("ε") != firstSet.end()) {
                auto followSet = computeFollow(lhs);
                for (auto x : followSet)
                    table[lhs][x] = prod;
            }
        }
    }
}

// Display Parsing Table
void displayParsingTable() {
    cout << "\n=== LL(1) Parsing Table ===\n";
    cout << setw(10) << "NT/T";
    for (auto t : terminals) if (t != "ε") cout << setw(10) << t;
    cout << setw(10) << "$\n";

    for (auto nt : nonTerminals) {
        cout << setw(10) << nt;
        for (auto t : terminals) {
            if (t == "ε") continue;
            if (!table[nt][t].empty()) {
                cout << setw(10);
                for (auto x : table[nt][t]) cout << x;
            } else cout << setw(10) << "-";
        }
        if (!table[nt]["$"].empty()) {
            cout << setw(10);
            for (auto x : table[nt]["$"]) cout << x;
        } else cout << setw(10) << "-";
        cout << "\n";
    }
}

int main() {
    int n;
    cout << "Enter number of productions: ";
    cin >> n;

    for (int i = 0; i < n; i++) {
        string line;
        cin >> line;
        string lhs = line.substr(0, line.find("->"));
        string rhs = line.substr(line.find("->") + 2);

        nonTerminals.insert(lhs);
        if (i == 0) startSymbol = lhs;

        stringstream ss(rhs);
        string prod;
        while (getline(ss, prod, '|')) {
            vector<string> symbols;
            for (int j = 0; j < prod.size(); j++) {
                string s(1, prod[j]);
                symbols.push_back(s);
                if (isTerminal(s) && s != "ε") terminals.insert(s);
            }
            grammar[lhs].push_back(symbols);
        }
    }

    // Compute FIRST and FOLLOW
    for (auto nt : nonTerminals) computeFirst(nt);
    for (auto nt : nonTerminals) computeFollow(nt);

    constructParsingTable();
    displayParsingTable();
}
