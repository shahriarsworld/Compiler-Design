#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <queue>

using namespace std;

struct Production {
    char lhs;
    string rhs;
    int prodNum;
};

struct Item {
    int prodNum;
    int dotPos;

    bool operator<(const Item& other) const {
        if (prodNum != other.prodNum) return prodNum < other.prodNum;
        return dotPos < other.dotPos;
    }

    bool operator==(const Item& other) const {
        return prodNum == other.prodNum && dotPos == other.dotPos;
    }
};

class LR0Parser {
private:
    vector<Production> productions;
    vector<set<Item>> states;
    map<pair<int, char>, int> transitions;
    vector<char> terminals;
    vector<char> nonTerminals;
    map<pair<int, char>, string> actionTable;
    map<pair<int, char>, int> gotoTable;
    char startSymbol;

    void extractSymbols() {
        set<char> terminalSet, nonTerminalSet;

        // Add start symbol to non-terminals
        nonTerminalSet.insert(startSymbol);
        nonTerminalSet.insert('S'); // Augmented start symbol

        // Extract all symbols from productions
        for (int i = 1; i < productions.size(); i++) { // Skip augmented production
            nonTerminalSet.insert(productions[i].lhs);
            for (char c : productions[i].rhs) {
                if (c >= 'A' && c <= 'Z') {
                    nonTerminalSet.insert(c);
                } else if (c != '|') {
                    terminalSet.insert(c);
                }
            }
        }

        // Convert sets to vectors
        terminals.assign(terminalSet.begin(), terminalSet.end());
        nonTerminals.assign(nonTerminalSet.begin(), nonTerminalSet.end());

        // Add $ to terminals
        terminals.push_back('$');

        // Sort for consistent output
        sort(terminals.begin(), terminals.end());
        sort(nonTerminals.begin(), nonTerminals.end());
    }

public:
    void inputGrammar() {
        productions.clear();
        terminals.clear();
        nonTerminals.clear();

        int numProductions;
        cout << "Enter the number of productions: ";
        cin >> numProductions;
        cin.ignore(); // Clear newline

        cout << "Enter the start symbol: ";
        cin >> startSymbol;
        cin.ignore();

        // Add augmented production S' -> S
        productions.push_back({'S', string(1, startSymbol), 0});

        cout << "Enter productions in format 'A->BC' or 'A->a|b' (one per line):\n";

        for (int i = 0; i < numProductions; i++) {
            string line;
            cout << "Production " << (i + 1) << ": ";
            getline(cin, line);

            // Remove spaces
            line.erase(remove(line.begin(), line.end(), ' '), line.end());

            // Find ->
            size_t arrowPos = line.find("->");
            if (arrowPos == string::npos) {
                cout << "Invalid format! Use A->BC format.\n";
                i--;
                continue;
            }

            char lhs = line[0];
            string rhs = line.substr(arrowPos + 2);

            // Handle alternatives separated by |
            stringstream ss(rhs);
            string alternative;
            while (getline(ss, alternative, '|')) {
                productions.push_back({lhs, alternative, (int)productions.size()});
            }
        }

        extractSymbols();

        cout << "\nAugmented Grammar:\n";
        for (int i = 0; i < productions.size(); i++) {
            if (i == 0) {
                cout << i << ": S' -> " << productions[i].rhs << "\n";
            } else {
                cout << i << ": " << productions[i].lhs << " -> " << productions[i].rhs << "\n";
            }
        }

        cout << "\nTerminals: {";
        for (int i = 0; i < terminals.size(); i++) {
            cout << terminals[i];
            if (i < terminals.size() - 1) cout << ", ";
        }
        cout << "}\n";

        cout << "Non-terminals: {";
        for (int i = 0; i < nonTerminals.size(); i++) {
            cout << nonTerminals[i];
            if (i < nonTerminals.size() - 1) cout << ", ";
        }
        cout << "}\n\n";
    }

    set<Item> closure(set<Item> items) {
        set<Item> result = items;
        bool changed = true;

        while (changed) {
            changed = false;
            set<Item> newItems = result;

            for (const Item& item : result) {
                if (item.dotPos < productions[item.prodNum].rhs.length()) {
                    char nextSymbol = productions[item.prodNum].rhs[item.dotPos];

                    // If next symbol is a non-terminal, add its productions
                    if (find(nonTerminals.begin(), nonTerminals.end(), nextSymbol) != nonTerminals.end()) {
                        for (int i = 0; i < productions.size(); i++) {
                            if (productions[i].lhs == nextSymbol) {
                                Item newItem = {i, 0};
                                if (newItems.find(newItem) == newItems.end()) {
                                    newItems.insert(newItem);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
            result = newItems;
        }
        return result;
    }

    set<Item> gotoState(set<Item> items, char symbol) {
        set<Item> result;

        for (const Item& item : items) {
            if (item.dotPos < productions[item.prodNum].rhs.length() &&
                productions[item.prodNum].rhs[item.dotPos] == symbol) {
                Item newItem = {item.prodNum, item.dotPos + 1};
                result.insert(newItem);
            }
        }

        return closure(result);
    }

    int findStateIndex(const set<Item>& state) {
        for (int i = 0; i < states.size(); i++) {
            if (states[i] == state) return i;
        }
        return -1;
    }

    void constructStates() {
        states.clear();
        transitions.clear();

        // Start with I0: closure of {S'->•startSymbol}
        set<Item> startState;
        startState.insert({0, 0}); // S'->•startSymbol
        startState = closure(startState);
        states.push_back(startState);

        queue<int> stateQueue;
        stateQueue.push(0);

        while (!stateQueue.empty()) {
            int currentStateIndex = stateQueue.front();
            stateQueue.pop();

            set<char> symbols;
            // Collect all symbols that can be shifted
            for (const Item& item : states[currentStateIndex]) {
                if (item.dotPos < productions[item.prodNum].rhs.length()) {
                    symbols.insert(productions[item.prodNum].rhs[item.dotPos]);
                }
            }

            for (char symbol : symbols) {
                set<Item> newState = gotoState(states[currentStateIndex], symbol);
                if (!newState.empty()) {
                    int newStateIndex = findStateIndex(newState);
                    if (newStateIndex == -1) {
                        states.push_back(newState);
                        newStateIndex = states.size() - 1;
                        stateQueue.push(newStateIndex);
                    }
                    transitions[{currentStateIndex, symbol}] = newStateIndex;
                }
            }
        }
    }

    void constructParsingTable() {
        actionTable.clear();
        gotoTable.clear();

        for (int i = 0; i < states.size(); i++) {
            for (const Item& item : states[i]) {
                if (item.dotPos == productions[item.prodNum].rhs.length()) {
                    // Reduce item
                    if (item.prodNum == 0) {
                        // Accept state
                        actionTable[{i, '$'}] = "acc";
                    } else {
                        // Reduce action
                        for (char terminal : terminals) {
                            actionTable[{i, terminal}] = "r" + to_string(item.prodNum);
                        }
                    }
                } else {
                    // Shift item
                    char nextSymbol = productions[item.prodNum].rhs[item.dotPos];
                    if (transitions.find({i, nextSymbol}) != transitions.end()) {
                        int nextState = transitions[{i, nextSymbol}];

                        if (find(terminals.begin(), terminals.end(), nextSymbol) != terminals.end()) {
                            actionTable[{i, nextSymbol}] = "s" + to_string(nextState);
                        } else {
                            gotoTable[{i, nextSymbol}] = nextState;
                        }
                    }
                }
            }
        }
    }

    void printStates() {
        cout << "LR(0) Collection of Items:\n\n";
        for (int i = 0; i < states.size(); i++) {
            cout << "I" << i << ":\n";
            for (const Item& item : states[i]) {
                cout << "  ";
                if (item.prodNum == 0) cout << "S'";
                else cout << productions[item.prodNum].lhs;
                cout << " -> ";

                string rhs = productions[item.prodNum].rhs;
                for (int j = 0; j <= rhs.length(); j++) {
                    if (j == item.dotPos) cout << "•";
                    if (j < rhs.length()) cout << rhs[j];
                }
                cout << "\n";
            }
            cout << "\n";
        }
    }

    bool checkLR0() {
        for (int i = 0; i < states.size(); i++) {
            map<char, int> actionCount;

            for (char terminal : terminals) {
                if (actionTable.find({i, terminal}) != actionTable.end()) {
                    actionCount[terminal]++;
                }
            }

            for (auto& count : actionCount) {
                if (count.second > 1) {
                    return false; // Conflict found
                }
            }
        }
        return true;
    }

    void printParsingTable() {
        cout << "LR(0) Parsing Table:\n\n";

        // Calculate column widths
        int stateWidth = 8;
        int actionWidth = 8;
        int gotoWidth = 8;

        cout << setw(stateWidth) << "STATE";

        // Print ACTION header
        cout << setw(actionWidth * terminals.size()) << "ACTION";
        cout << setw(gotoWidth * nonTerminals.size()) << "GOTO";
        cout << "\n";

        cout << setw(stateWidth) << "";
        for (char terminal : terminals) {
            cout << setw(actionWidth) << terminal;
        }
        for (char nonTerminal : nonTerminals) {
            cout << setw(gotoWidth) << nonTerminal;
        }
        cout << "\n";

        // Print separator line
        cout << string(stateWidth + actionWidth * terminals.size() + gotoWidth * nonTerminals.size(), '-') << "\n";

        for (int i = 0; i < states.size(); i++) {
            cout << setw(stateWidth) << i;

            // Print ACTION columns
            for (char terminal : terminals) {
                if (actionTable.find({i, terminal}) != actionTable.end()) {
                    cout << setw(actionWidth) << actionTable[{i, terminal}];
                } else {
                    cout << setw(actionWidth) << "";
                }
            }

            // Print GOTO columns
            for (char nonTerminal : nonTerminals) {
                if (gotoTable.find({i, nonTerminal}) != gotoTable.end()) {
                    cout << setw(gotoWidth) << gotoTable[{i, nonTerminal}];
                } else {
                    cout << setw(gotoWidth) << "";
                }
            }
            cout << "\n";
        }

        cout << "\nLegend:\n";
        cout << "sN = shift to state N\n";
        cout << "rN = reduce by production N\n";
        cout << "acc = accept\n";
        cout << "blank = error\n\n";

        cout << "Productions:\n";
        for (int i = 0; i < productions.size(); i++) {
            if (i == 0) {
                cout << i << ": S' -> " << productions[i].rhs << "\n";
            } else {
                cout << i << ": " << productions[i].lhs << " -> " << productions[i].rhs << "\n";
            }
        }
    }

    void run() {
        cout << "=== LR(0) Parser Constructor ===\n\n";

        inputGrammar();

        cout << "Constructing LR(0) collection of items...\n";
        constructStates();

        cout << "Constructing parsing table...\n";
        constructParsingTable();

        printStates();
        printParsingTable();

        if (checkLR0()) {
            cout << "\n✓ The grammar is LR(0) since each cell in the parsing table contains at most one entry.\n";
        } else {
            cout << "\n✗ The grammar is NOT LR(0) due to shift-reduce or reduce-reduce conflicts.\n";
        }
    }
};

int main() {
    LR0Parser parser;
    parser.run();
    return 0;
}
