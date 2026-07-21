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

bool is_numeric(const string &str) {
    if (str.empty()) return false;
    int start = 0;
    if (str[0] == '-') {
        start = 1;
        if (str.length() == start) {
            return false;
        }
    }
    for (int i = start; i < str.length(); ++i) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

void int2hex16(char *lower, int a) {
    sprintf(lower, "%04X", a & 0xFFFF);
}

int hex2int(char *hex) {
    return (int) strtol(hex, NULL, 16);
}

Instruction parse_line(const string &line) {
    Instruction inst;
    inst.arg_count = 0;

    size_t comment_pos = line.find('#');
    string clean;
    if (comment_pos != string::npos) {
        clean = line.substr(0, comment_pos);
    } else {
        clean = line;
    }

    int start = 0;
    while (start < (int) clean.length() &&
           (clean[start] == ' ' || clean[start] == '\t' || clean[start] == '\n' || clean[start] == '\r')) {
        start++;
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

    char *tokens[6];
    int t_cnt = 0;

    char *token = strtok(line_copy, " \t");
    while (token != NULL && t_cnt < 6) {
        tokens[t_cnt++] = token;
        token = strtok(NULL, " \t");
    }

    if (t_cnt == 0) return inst;

    string ops[] = {"add", "sub", "slt", "or", "nand", "addi", "slti", "ori", "lui", "lw", "sw", "beq", "jalr", "j",
                    "halt", ".fill", ".space"};

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
            for (int i = 0; i < idx; i++) {
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

int get_symbol_address(struct symbolTable *symT, int len, const string &name) {
    for (int i = 0; i < len; ++i) {
        if (string(symT[i].name) == name) {
            return symT[i].address;
        }
    }
    return -1;
}

void pass2(FILE *inputFile, FILE *outputFile, struct symbolTable *symT, int symLen) {
    char buffer[256];
    int pc = 0;

    while (fgets(buffer, sizeof(buffer), inputFile)) {
        string line(buffer);
        Instruction inst = parse_line(line);

        if (inst.opcode.empty() && inst.label.empty()) continue;
        if (inst.opcode.empty()) continue;

        if (inst.opcode == ".space") {
            int spaces = stoi(inst.args[0]);
            for (int i = 0; i < spaces; i++) {
                fprintf(outputFile, "0\n");
            }
            pc += spaces;
            continue;
        }

        if (inst.opcode == ".fill") {
            int val;
            if (is_numeric(inst.args[0])) {
                val = stoi(inst.args[0]);
            } else {
                val = get_symbol_address(symT, symLen, inst.args[0]);
                if (val == -1) exit(1);
            }
            fprintf(outputFile, "%d\n", val);
            pc++;
            continue;
        }

        int op = -1, type = -1;
        if (inst.opcode == "add") {
            op = 0;
            type = 0;
        } else if (inst.opcode == "sub") {
            op = 1;
            type = 0;
        } else if (inst.opcode == "slt") {
            op = 2;
            type = 0;
        } else if (inst.opcode == "or") {
            op = 3;
            type = 0;
        } else if (inst.opcode == "nand") {
            op = 4;
            type = 0;
        } else if (inst.opcode == "addi") {
            op = 5;
            type = 1;
        } else if (inst.opcode == "slti") {
            op = 6;
            type = 1;
        } else if (inst.opcode == "ori") {
            op = 7;
            type = 1;
        } else if (inst.opcode == "lui") {
            op = 8;
            type = 1;
        } else if (inst.opcode == "lw") {
            op = 9;
            type = 1;
        } else if (inst.opcode == "sw") {
            op = 10;
            type = 1;
        } else if (inst.opcode == "beq") {
            op = 11;
            type = 1;
        } else if (inst.opcode == "jalr") {
            op = 12;
            type = 1;
        } else if (inst.opcode == "j") {
            op = 13;
            type = 2;
        } else if (inst.opcode == "halt") {
            op = 14;
            type = 2;
        } else {
            exit(1);
        }

        char hex_str[9] = "00000000";

        if (type == 0) {
            if (inst.arg_count < 3) exit(1);
            int rd = stoi(inst.args[0]);
            int rs = stoi(inst.args[1]);
            int rt = stoi(inst.args[2]);
            sprintf(hex_str, "0%X%X%X%X000", op, rs, rt, rd);
            int machine_code = hex2int(hex_str);
            fprintf(outputFile, "%d\n", machine_code);
            pc++;
        } else if (type == 1) {
            if (inst.arg_count < 2 && inst.opcode != "jalr") exit(1);
            int rt = stoi(inst.args[0]);
            int rs = 0;
            int offset = 0;

            if (inst.opcode == "lui") {
                if (is_numeric(inst.args[1])) {
                    offset = stoi(inst.args[1]);
                } else {
                    offset = get_symbol_address(symT, symLen, inst.args[1]);
                    if (offset == -1) exit(1);
                }
            } else if (inst.opcode == "jalr") {
                if (inst.arg_count >= 2) {
                    rs = stoi(inst.args[1]);
                }
                offset = 0;
            } else {
                if (inst.arg_count < 3) exit(1);
                rs = stoi(inst.args[1]);
                string target = inst.args[2];
                if (is_numeric(target)) {
                    offset = stoi(target);
                } else {
                    int addr = get_symbol_address(symT, symLen, target);
                    if (addr == -1) exit(1);
                    if (inst.opcode == "beq") {
                        offset = addr - pc - 1;
                    } else {
                        offset = addr;
                    }
                }
            }

            if (offset < -32768 || offset > 65535) exit(1);
            char lower[5];
            int2hex16(lower, offset);
            sprintf(hex_str, "0%X%X%X%s", op, rs, rt, lower);
            int machine_code = hex2int(hex_str);
            fprintf(outputFile, "%d\n", machine_code);
            pc++;
        } else if (type == 2) {
            int offset = 0;
            if (inst.opcode != "halt") {
                if (inst.arg_count == 0) exit(1);
                if (is_numeric(inst.args[0])) {
                    offset = stoi(inst.args[0]);
                } else {
                    offset = get_symbol_address(symT, symLen, inst.args[0]);
                    if (offset == -1) exit(1);
                }
                if (offset < -32768 || offset > 65535) exit(1);
            }
            char lower[5];
            int2hex16(lower, offset);
            sprintf(hex_str, "0%X00%s", op, lower);
            int machine_code = hex2int(hex_str);
            fprintf(outputFile, "%d\n", machine_code);
            pc++;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " input.asm output.obj" << endl;
        exit(1);
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        cerr << "Error: Cannot open input file: " << argv[1] << endl;
        exit(1);
    }

    FILE *fout = fopen(argv[2], "w");
    if (!fout) {
        cerr << "Error: Cannot create output file: " << argv[2] << endl;
        fclose(fin);
        exit(1);
    }

    int symLen = findSymTabLen(fin);
    struct symbolTable *symT = (struct symbolTable *) malloc(symLen * sizeof(struct symbolTable));
    if (symLen > 0 && !symT) {
        cerr << "Error: Memory allocation failed" << endl;
        fclose(fin);
        fclose(fout);
        exit(1);
    }

    fillSymTab(symT, fin);
    pass2(fin, fout, symT, symLen);

    free(symT);
    fclose(fin);
    fclose(fout);

    cout << "Assembly completed successfully!" << endl;
    return 0;
}