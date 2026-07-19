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

bool is_numeric(const string& str) {
    if (str.empty()) return false;
    size_t start ;
    if (str[0] == '-' || str[0] == '+'){
        start =1;
    }else{
        start =0;
    }
    if (start == str.length()){
        return false;
    }
    for (size_t i = start; i < str.length(); ++i) {
        if (!isdigit(static_cast<unsigned char>(str[i]))){
            return false;
        }
    }
    return true;
}

void int2hex16(char *lower, int a) {
    sprintf(lower, "%04X", a & 0xFFFF);
}

int hex2int(char *hex) {
    return (int)strtol(hex, NULL, 16);
}


int main() {


    return 0;
}