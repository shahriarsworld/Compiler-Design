#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
using namespace std;


vector<string> loadFromFile(const string& filename) {
    vector<string> list;
    ifstream file(filename);
    string word;

    while (getline(file, word)) {
        if (!word.empty()) list.push_back(word);
    }

    return list;
}

// Check if token is in a given list
bool isInList(const string& token, const vector<string>& list) {
    for (const string& item : list) {
        if (token == item) return true;
    }
    return false;
}

// Check if token is a number
bool isNumber(const string& token) {
    if (token.empty()) return false;
    for (char c : token) {
        if (!isdigit(c) && c != '.') return false;
    }
    return true;
}

// Clean trailing and leading punctuation
string cleanToken(string token) {
    // Remove leading symbols
    while (!token.empty() && ispunct(token.front()) && token.front() != '_' && token.front() != '#') {
        token.erase(0, 1);
    }

    // Remove trailing symbols (but keep . and _ for things like 3.14 or my_var)
    while (!token.empty() && ispunct(token.back()) && token.back() != '.' && token.back() != '_') {
        token.pop_back();
    }

    return token;
}

int main() {
    // Load keywords and operators from file
    vector<string> keywords = loadFromFile("keywords.txt");
    vector<string> operators = loadFromFile("operators.txt");

    ifstream code("code.txt");
    if (!code.is_open()) {
        cout << "Could not open code.txt" << endl;
        return 1;
    }

    string line;
    while (getline(code, line)) {
        stringstream ss(line);
        string token;

        while (ss >> token) {
            token = cleanToken(token);
            if (token.empty()) continue;

            if (isInList(token, keywords)) {
                cout << token << " -> Keyword" << endl;
            }
            else if (isInList(token, operators)) {
                cout << token << " -> Operator" << endl;
            }
            else if (isNumber(token)) {
                cout << token << " -> Number" << endl;
            }
            else {
                cout << token << " -> Identifier" << endl;
            }
        }
    }

    code.close();
    return 0;
}
