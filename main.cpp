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

int fillSymTab(struct symbolTable *symT, FILE *inputFile) {
    int pc = 0;
    int idx = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), inputFile)) {
        string line(buffer);
        Instruction inst = parse_line(line);

        if (inst.opcode.empty() && inst.label.empty()) continue;

        if (!inst.label.empty()) {
            for(int i = 0; i < idx; i++) {
                if (string(symT[i].name) == inst.label) {
                    exit(1);
                }
            }
            strncpy(symT[idx].name, inst.label.c_str(), 6);
            symT[idx].name[6] = '\0';
            symT[idx].address = pc;
            idx++;
        }

        if (inst.opcode == ".space") {
            if (inst.arg_count == 0 || !is_numeric(inst.args[0])) {
                exit(1);
            }
            pc += stoi(inst.args[0]);
        } else if (!inst.opcode.empty()) {
            pc++;
        }
    }
    rewind(inputFile);
    return idx;
}

int get_symbol_address(struct symbolTable *symT, int len, const string& name) {
    for (int i = 0; i < len; ++i) {
        if (string(symT[i].name) == name) {
            return symT[i].address;
        }
    }
    return -1;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " input.asm output.obj" << endl;
        exit(1);
    }

    FILE* fin = fopen(argv[1], "r");
    if (!fin) {
        cerr << "Error: Cannot open input file: " << argv[1] << endl;
        exit(1);
    }

    int symLen = findSymTabLen(fin);
    struct symbolTable* symT = (struct symbolTable*)malloc(symLen * sizeof(struct symbolTable));
    if (symLen > 0 && !symT) {
        cerr << "Error: Memory allocation failed" << endl;
        fclose(fin);
        exit(1);
    }

    fillSymTab(symT, fin);

    for (int i = 0; i < symLen; i++) {
        cout << "Symbol: " << symT[i].name << " Address: " << symT[i].address << endl;
    }

    free(symT);
    fclose(fin);
    return 0;
}