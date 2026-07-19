using namespace std;

#include <iostream>
#include <string>
#include <cstring>


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

Instruction parse_line(const string& line) {
    Instruction inst;
    inst.arg_count = 0;

    size_t comment_pos = line.find('#');
    string clean;
    if (comment_pos != string::npos) {
        clean = line.substr(0, comment_pos);
    } else {
        clean = line;
    }

    size_t start = clean.find_first_not_of(" \t\n\r");
    if (start == string::npos) {
        return inst;
    }
    clean = clean.substr(start);

    size_t end = clean.find_last_not_of(" \t\n\r");
    if (end != string::npos) {
        clean = clean.substr(0, end + 1);
    }

    for (size_t i = 0; i < clean.length(); ++i) {
        if (clean[i] == ',') clean[i] = ' ';
    }

    char line_copy[256];
    strcpy(line_copy, clean.c_str());

    char* tokens[6];
    int t_cnt = 0;

    char* token = strtok(line_copy, " \t");
    while (token != NULL && t_cnt < 6) {
        tokens[t_cnt++] = token;
        token = strtok(NULL, " \t");
    }

    if (t_cnt == 0) return inst;

    string ops[] = {"add","sub","slt","or","nand","addi","slti","ori","lui","lw","sw","beq","jalr","j","halt",".fill",".space"};

    bool is_op = false;
    for (int i = 0; i < 17; ++i) {
        if (string(tokens[0]) == ops[i]) {
            is_op = true;
            break;
        }
    }

    int arg_start = 0;
    if (is_op) {
        inst.opcode = tokens[0];
        arg_start = 1;
    } else {
        inst.label = tokens[0];
        if (t_cnt > 1) {
            inst.opcode = tokens[1];
            arg_start = 2;
        }
    }

    for (int i = arg_start; i < t_cnt && inst.arg_count < 3; ++i) {
        inst.args[inst.arg_count++] = tokens[i];
    }

    return inst;
}

int findSymTabLen(FILE *inputFile) {
    int count = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), inputFile)) {
        string line(buffer);
        Instruction inst = parse_line(line);
        if (!inst.label.empty()) count++;
    }
    rewind(inputFile);
    return count;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " input.asm output.obj" << endl;
        exit(1);
    }

    cout << "Assembler started..." << endl;
    return 0;
}