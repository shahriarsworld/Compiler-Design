#include <iostream>
#include <fstream>
using namespace std;

int main(){
ofstream file("diary.txt");

if (file.is_open()){
    file<< "Date: 19/06/2025 \n";
    file<< "Today I am learning about File Handling in C++. \n";
    file<< "I find this topic interesting.";
    file.close();
    cout<< "Diary written succesfully";
} else{
    cout<< "Failed to open file";
}

return 0;
}
