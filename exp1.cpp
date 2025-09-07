#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main() {
    string file_path = "storage.txt";
    string user_input;
    int option;

    cout << "Choose an action:\n";
    cout << "1) Save to file\n";
    cout << "2) View file contents\n";
    cout << "3) Add to file\n";
    cout << "Enter choice (1-3): ";
    cin >> option;
    cin.ignore();

    switch (option) {
        case 1: {
            ofstream writer(file_path, ios::out);
            if (!writer) {
                cout << "Failed to open file for writing.\n";
                return 1;
            }
            cout << "Enter data to save: ";
            getline(cin, user_input);
            writer << user_input << '\n';
            writer.close();
            cout << "Data saved successfully.\n";
            break;
        }
        case 2: {
            ifstream reader(file_path, ios::in);
            if (!reader) {
                cout << "Cannot open file. It may not exist.\n";
                return 1;
            }
            cout << "File contents:\n";
            while (getline(reader, user_input)) {
                cout << user_input << '\n';
            }
            reader.close();
            break;
        }
        case 3: {
            ofstream appender(file_path, ios::app);
            if (!appender) {
                cout << "Failed to open file for appending.\n";
                return 1;
            }
            cout << "Enter data to add: ";
            getline(cin, user_input);
            appender << user_input << '\n';
            appender.close();
            cout << "Data added successfully.\n";
            break;
        }
        default:
            cout << "Invalid option. Choose 1, 2, or 3.\n";
    }
    return 0;
}
