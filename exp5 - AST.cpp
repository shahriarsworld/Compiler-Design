#include <bits/stdc++.h>
using namespace std;

// AST Node structure
struct Node {
    string value;
    Node *left, *right;
    Node(string val) {
        value = val;
        left = right = nullptr;
    }
};

// Operator precedence
int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

// Convert infix to postfix using Shunting Yard
vector<string> infixToPostfix(string expr) {
    stack<char> st;
    vector<string> output;
    for (int i = 0; i < expr.size(); i++) {
        char c = expr[i];

        if (isspace(c)) continue;

        if (isalnum(c)) {
            string operand(1, c);
            output.push_back(operand);
        }
        else if (c == '(') {
            st.push(c);
        }
        else if (c == ')') {
            while (!st.empty() && st.top() != '(') {
                output.push_back(string(1, st.top()));
                st.pop();
            }
            st.pop();
        }
        else { // operator
            while (!st.empty() && precedence(st.top()) >= precedence(c)) {
                output.push_back(string(1, st.top()));
                st.pop();
            }
            st.push(c);
        }
    }
    while (!st.empty()) {
        output.push_back(string(1, st.top()));
        st.pop();
    }
    return output;
}

// Build AST from postfix
Node* buildAST(vector<string> &postfix) {
    stack<Node*> st;
    for (auto &token : postfix) {
        if (isalnum(token[0])) {
            st.push(new Node(token));
        } else {
            Node* node = new Node(token);
            node->right = st.top(); st.pop();
            node->left = st.top(); st.pop();
            st.push(node);
        }
    }
    return st.top();
}

// Generate Assembly Code
int regCount = 0;
string newReg() {
    return "R" + to_string(regCount++);
}

string generateCode(Node* root) {
    if (!root) return "";

    // Operand
    if (!root->left && !root->right) {
        string r = newReg();
        cout << "LOAD " << root->value << ", " << r << "\n";
        return r;
    }

    // Recurse for left and right
    string leftReg = generateCode(root->left);
    string rightReg = generateCode(root->right);
    string resultReg = newReg();

    if (root->value == "+")
        cout << "ADD " << leftReg << ", " << rightReg << ", " << resultReg << "\n";
    else if (root->value == "-")
        cout << "SUB " << leftReg << ", " << rightReg << ", " << resultReg << "\n";
    else if (root->value == "*")
        cout << "MUL " << leftReg << ", " << rightReg << ", " << resultReg << "\n";
    else if (root->value == "/")
        cout << "DIV " << leftReg << ", " << rightReg << ", " << resultReg << "\n";

    return resultReg;
}

int main() {
    string expr;
    cout << "Enter arithmetic expression: ";
    getline(cin, expr);

    // Step 1: Infix to Postfix
    vector<string> postfix = infixToPostfix(expr);

    // Step 2: Build AST
    Node* root = buildAST(postfix);

    // Step 3: Generate Assembly
    cout << "\n=== Target Assembly Code ===\n";
    string finalReg = generateCode(root);
    cout << "STORE " << finalReg << ", RESULT\n";
}

