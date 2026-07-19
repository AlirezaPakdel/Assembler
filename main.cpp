using namespace std;

#include <iostream>
#include <string>




struct symbolTable {
    char name[7];
    int address;
};

struct Instruction {
    string label;
    string opcode;
    string args[3];
    int arg_count;
};

int main() {


    return 0;
}