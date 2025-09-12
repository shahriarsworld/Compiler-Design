#include <bits/stdc++.h>
using namespace std;

struct Item {
    string lhs;
    vector<string> rhs;
    int dotPos;
    string lookahead;
};

struct State {
    vector<Item> items;
    map<string,int> transitions;
};

map<string, vector<vector<string>>> grammar;
vector<State> states;
map<pair<int,string>,string> ACTION;  // ACTION table
map<pair<int,string>,int> GOTO;      // GOTO table
set<string> terminals, nonTerminals;
string startSymbol;

// Utility
bool isTerminal(const string &sym) {
    return !(sym[0]>='A' && sym[0]<='Z');
}

// FIRST set (for lookahead propagation)
map<string,set<string>> FIRST;

set<string> computeFirst(const vector<string> &alpha) {
    set<string> result;
    bool eps = true;
    for (auto X : alpha) {
        if (isTerminal(X)) {
            result.insert(X);
            eps = false;
            break;
        } else {
            for (auto y : FIRST[X]) if (y!="ε") result.insert(y);
            if (FIRST[X].find("ε")==FIRST[X].end()) {
                eps = false; break;
            }
        }
    }
    if (eps) result.insert("ε");
    return result;
}

set<string> firstOfSymbol(const string &X) {
    if (isTerminal(X)) return {X};
    return FIRST[X];
}

// Compute FIRST sets for grammar
void buildFirstSets() {
    bool changed=true;
    while (changed) {
        changed=false;
        for (auto &[lhs,prods]:grammar) {
            for (auto prod:prods) {
                set<string> f=computeFirst(prod);
                for (auto x:f) {
                    if (FIRST[lhs].insert(x).second) changed=true;
                }
            }
        }
    }
}

// Check if two items are equal
bool equalItem(const Item &a,const Item &b) {
    return a.lhs==b.lhs && a.rhs==b.rhs && a.dotPos==b.dotPos && a.lookahead==b.lookahead;
}

// Closure operation for LR(1)
vector<Item> closure(vector<Item> I) {
    bool added=true;
    while (added) {
        added=false;
        vector<Item> newItems;
        for (auto &it:I) {
            if (it.dotPos<it.rhs.size()) {
                string B=it.rhs[it.dotPos];
                if (!isTerminal(B)) {
                    vector<string> beta;
                    for (int k=it.dotPos+1;k<it.rhs.size();k++) beta.push_back(it.rhs[k]);
                    beta.push_back(it.lookahead);
                    set<string> lookaheads=computeFirst(beta);
                    for (auto prod:grammar[B]) {
                        for (auto a:lookaheads) {
                            if (a=="ε") continue;
                            Item newItem={B,prod,0,a};
                            bool exists=false;
                            for (auto &p:I) if (equalItem(p,newItem)) exists=true;
                            for (auto &p:newItems) if (equalItem(p,newItem)) exists=true;
                            if (!exists) {
                                newItems.push_back(newItem);
                                added=true;
                            }
                        }
                    }
                }
            }
        }
        I.insert(I.end(),newItems.begin(),newItems.end());
    }
    return I;
}

// GOTO operation
vector<Item> GOTOfunc(vector<Item> I,string X) {
    vector<Item> J;
    for (auto &it:I) {
        if (it.dotPos<it.rhs.size() && it.rhs[it.dotPos]==X) {
            Item moved=it;
            moved.dotPos++;
            J.push_back(moved);
        }
    }
    return closure(J);
}

// Compare states
bool sameState(const vector<Item> &a,const vector<Item> &b) {
    if (a.size()!=b.size()) return false;
    for (auto &x:a) {
        bool found=false;
        for (auto &y:b) if (equalItem(x,y)) found=true;
        if (!found) return false;
    }
    return true;
}

// Build Canonical Collection
void buildCLR1Automaton() {
    Item start={startSymbol+"'",{startSymbol},0,"$"};
    vector<Item> startState=closure({start});
    states.push_back({startState,{}});
    for (int i=0;i<states.size();i++) {
        set<string> symbols;
        for (auto &it:states[i].items)
            if (it.dotPos<it.rhs.size()) symbols.insert(it.rhs[it.dotPos]);
        for (auto X:symbols) {
            vector<Item> newState=GOTOfunc(states[i].items,X);
            if (newState.empty()) continue;
            bool exists=false; int idx=-1;
            for (int j=0;j<states.size();j++) {
                if (sameState(states[j].items,newState)) {
                    exists=true; idx=j; break;
                }
            }
            if (!exists) {
                states.push_back({newState,{}});
                idx=states.size()-1;
            }
            states[i].transitions[X]=idx;
        }
    }
}

// Build ACTION and GOTO table
void constructParsingTable() {
    for (int i=0;i<states.size();i++) {
        for (auto &it:states[i].items) {
            if (it.dotPos<it.rhs.size()) {
                string a=it.rhs[it.dotPos];
                if (isTerminal(a)) {
                    int j=states[i].transitions[a];
                    ACTION[{i,a}]="s"+to_string(j);
                } else {
                    int j=states[i].transitions[a];
                    GOTO[{i,a}]=j;
                }
            } else {
                if (it.lhs==startSymbol+"'") {
                    ACTION[{i,"$"}]="acc";
                } else {
                    string prod=it.lhs+"->";
                    for (auto x:it.rhs) prod+=x;
                    ACTION[{i,it.lookahead}]="r("+prod+")";
                }
            }
        }
    }
}

// Display table
void displayTable() {
    cout<<"\n=== CLR(1) Parsing Table ===\n";
    cout<<setw(8)<<"State";
    for (auto t:terminals) cout<<setw(8)<<t;
    cout<<setw(8)<<"$";
    for (auto nt:nonTerminals) cout<<setw(8)<<nt;
    cout<<"\n";
    for (int i=0;i<states.size();i++) {
        cout<<setw(8)<<i;
        for (auto t:terminals) {
            if (ACTION[{i,t}]!="") cout<<setw(8)<<ACTION[{i,t}];
            else cout<<setw(8)<<"-";
        }
        if (ACTION[{i,"$"}]!="") cout<<setw(8)<<ACTION[{i,"$"}];
        else cout<<setw(8)<<"-";
        for (auto nt:nonTerminals) {
            if (GOTO[{i,nt}]!=0) cout<<setw(8)<<GOTO[{i,nt}];
            else cout<<setw(8)<<"-";
        }
        cout<<"\n";
    }
}

int main() {
    int n;
    cout<<"Enter number of productions: ";
    cin>>n;
    for (int i=0;i<n;i++) {
        string line; cin>>line;
        string lhs=line.substr(0,line.find("->"));
        string rhs=line.substr(line.find("->")+2);
        nonTerminals.insert(lhs);
        if (i==0) startSymbol=lhs;
        stringstream ss(rhs);
        string prod;
        while (getline(ss,prod,'|')) {
            vector<string> symbols;
            for (int j=0;j<prod.size();j++) {
                string s(1,prod[j]);
                symbols.push_back(s);
                if (isTerminal(s) && s!="ε") terminals.insert(s);
            }
            grammar[lhs].push_back(symbols);
        }
    }
    buildFirstSets();
    buildCLR1Automaton();
    constructParsingTable();
    displayTable();
}

