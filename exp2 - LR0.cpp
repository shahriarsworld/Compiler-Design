#include <bits/stdc++.h>
using namespace std;

struct Item {
    string lhs;
    vector<string> rhs;
    int dotPos;
};

struct State {
    vector<Item> items;
    map<string, int> transitions;
};

map<string, vector<vector<string>>> grammar;
vector<State> states;
map<pair<int, string>, string> ACTION; // ACTION table
map<pair<int, string>, int> GOTO;      // GOTO table
set<string> terminals, nonTerminals;
string startSymbol;

// Check if symbol is terminal
bool isTerminal(const string &sym) {
    return !(sym[0] >= 'A' && sym[0] <= 'Z');
}

// Closure operation
vector<Item> closure(vector<Item> I) {
    bool added = true;
    while (added) {
        added = false;
        vector<Item> newItems;
        for (auto &item : I) {
            if (item.dotPos < item.rhs.size()) {
                string B = item.rhs[item.dotPos];
                if (!isTerminal(B)) {
                    for (auto &prod : grammar[B]) {
                        Item newItem = {B, prod, 0};
                        bool exists = false;
                        for (auto &it : I)
                            if (it.lhs == newItem.lhs && it.rhs == newItem.rhs && it.dotPos == newItem.dotPos)
                                exists = true;
                        for (auto &it : newItems)
                            if (it.lhs == newItem.lhs && it.rhs == newItem.rhs && it.dotPos == newItem.dotPos)
                                exists = true;
                        if (!exists) {
                            newItems.push_back(newItem);
                            added = true;
                        }
                    }
                }
            }
        }
        I.insert(I.end(), newItems.begin(), newItems.end());
    }
    return I;
}

// GOTO operation
vector<Item> GOTOfunc(vector<Item> I, string X) {
    vector<Item> J;
    for (auto &item : I) {
        if (item.dotPos < item.rhs.size() && item.rhs[item.dotPos] == X) {
            Item moved = item;
            moved.dotPos++;
            J.push_back(moved);
        }
    }
    return closure(J);
}

// Compare states
bool sameState(vector<Item> &a, vector<Item> &b) {
    if (a.size() != b.size()) return false;
    for (auto &x : a) {
        bool found = false;
        for (auto &y : b)
            if (x.lhs == y.lhs && x.rhs == y.rhs && x.dotPos == y.dotPos)
                found = true;
        if (!found) return false;
    }
    return true;
}

// Build Canonical Collection
void buildLR0Automaton() {
    Item startItem = {startSymbol + "'", {startSymbol}, 0};
    vector<Item> startState = closure({startItem});
    states.push_back({startState, {}});

    for (int i = 0; i < states.size(); i++) {
        set<string> symbols;
        for (auto &item : states[i].items) {
            if (item.dotPos < item.rhs.size()) symbols.insert(item.rhs[item.dotPos]);
        }
        for (auto X : symbols) {
            vector<Item> newState = GOTOfunc(states[i].items, X);
            if (newState.empty()) continue;
            bool exists = false;
            int index = -1;
            for (int j = 0; j < states.size(); j++) {
                if (sameState(states[j].items, newState)) {
                    exists = true;
                    index = j;
                    break;
                }
            }
            if (!exists) {
                states.push_back({newState, {}});
                index = states.size() - 1;
            }
            states[i].transitions[X] = index;
        }
    }
}

// Construct ACTION and GOTO table
void constructParsingTable() {
    for (int i = 0; i < states.size(); i++) {
        for (auto &item : states[i].items) {
            if (item.dotPos < item.rhs.size()) {
                string a = item.rhs[item.dotPos];
                if (isTerminal(a)) {
                    int j = states[i].transitions[a];
                    ACTION[{i, a}] = "s" + to_string(j); // shift
                } else {
                    int j = states[i].transitions[a];
                    GOTO[{i, a}] = j;
                }
            } else {
                if (item.lhs == startSymbol + "'") {
                    ACTION[{i, "$"}] = "acc"; // accept
                } else {
                    string prod = item.lhs + "->";
                    for (auto x : item.rhs) prod += x;
                    for (auto t : terminals) {
                        ACTION[{i, t}] = "r(" + prod + ")";
                    }
                    ACTION[{i, "$"}] = "r(" + prod + ")";
                }
            }
        }
    }
}

// Display Parsing Table
void displayTable() {
    cout << "\n=== LR(0) Parsing Table ===\n";
    cout << setw(8) << "State";
    for (auto t : terminals) cout << setw(8) << t;
    cout << setw(8) << "$";
    for (auto nt : nonTerminals) cout << setw(8) << nt;
    cout << "\n";

    for (int i = 0; i < states.size(); i++) {
        cout << setw(8) << i;
        for (auto t : terminals) {
            if (ACTION[{i, t}] != "")
                cout << setw(8) << ACTION[{i, t}];
            else
                cout << setw(8) << "-";
        }
        if (ACTION[{i, "$"}] != "")
            cout << setw(8) << ACTION[{i, "$"}];
        else
            cout << setw(8) << "-";
        for (auto nt : nonTerminals) {
            if (GOTO[{i, nt}] != 0)
                cout << setw(8) << GOTO[{i, nt}];
            else
                cout << setw(8) << "-";
        }
        cout << "\n";
    }
}

int main() {
    int n;
    cout << "Enter number of productions: ";
    cin >> n;
    cout << "Format: E->E+T | T (no spaces)\n";

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

    buildLR0Automaton();
    constructParsingTable();
    displayTable();
}
