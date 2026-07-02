#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdint>
#include <iomanip>
using namespace std;

// =============================================================================
// SECTION 1 - CUSTOM DATA STRUCTURES
// =============================================================================

class MyList {
private:
    string* data;
    int capacity;
    int size;

    void resize() {
        capacity *= 2;
        string* newData = new string[capacity];
        for (int i = 0; i < size; i++) newData[i] = data[i];
        delete[] data;
        data = newData;
    }

public:
    MyList() {
        capacity = 16;
        size = 0;
        data = new string[capacity];
    }
    ~MyList() { delete[] data; }

    void push_back(const string& val) {
        if (size == capacity) resize();
        data[size++] = val;
    }

    string get(int index) const { return data[index]; }
    void set(int index, const string& val) { data[index] = val; }
    int getSize() const { return size; }
    bool isEmpty() const { return size == 0; }
};

// section 2 
class Register {
protected:
    int8_t value; 

public:
    Register() : value(0) {}
    virtual ~Register() {}
    virtual int8_t getValue() const { return value; }
    virtual void setValue(int8_t v) { value = v; }
    virtual void reset() { value = 0; }
};

class GeneralRegister : public Register {
private:
    int id; 

public:
    GeneralRegister(int id = 0) : Register(), id(id) {}
    int getId() const { return id; }
    void setValue(int8_t v) override { value = v; }
};

class FlagRegister {
private:
    int OF; 
    int UF; 
    int CF; 
    int ZF; 

public:
    FlagRegister() : OF(0), UF(0), CF(0), ZF(0) {}

    int getOF() const { return OF; }
    int getUF() const { return UF; }
    int getCF() const { return CF; }
    int getZF() const { return ZF; }

    void setOF(int v) { OF = (v ? 1 : 0); }
    void setUF(int v) { UF = (v ? 1 : 0); }
    void setCF(int v) { CF = (v ? 1 : 0); }
    void setZF(int v) { ZF = (v ? 1 : 0); }

    void resetAll() { OF = UF = CF = ZF = 0; }

    void updateFlags(int fullResult, bool arithmeticOp) {
        OF = (fullResult > 127)  ? 1 : 0;
        UF = (fullResult < -128) ? 1 : 0;
        ZF = (fullResult == 0)   ? 1 : 0;
        if (arithmeticOp) {
            CF = (fullResult > 255 || fullResult < 0) ? 1 : 0;
        }
    }
};

// Section 4
class CPU;

class Instruction {
protected:
    string rawLine;

public:
    Instruction(const string& line) : rawLine(line) {}
    virtual ~Instruction() {}
    virtual void execute(CPU& cpu) = 0;
    string getRaw() const { return rawLine; }
};

// SECTION 5 - CPU CLASS
class CPU {
private:
    GeneralRegister registers[8]; 
    Memory memory;                
    FlagRegister flags;           
    MyStack stack;                
    uint8_t PC;                    
    uint8_t SI;                    

public:
    CPU() : PC(0), SI(0) {
        for (int i = 0; i < 8; i++) {
            registers[i] = GeneralRegister(i);
        }
    }

    int8_t getRegister(int index) const { return registers[index].getValue(); }
    void setRegister(int index, int8_t val) { registers[index].setValue(val); }

    int8_t readMemory(int addr) const { return memory.read(addr); }
    void writeMemory(int addr, int8_t val) { memory.write(addr, val); }

    FlagRegister& getFlags() { return flags; }
    const FlagRegister& getFlags() const { return flags; }

    uint8_t getPC() const { return PC; }
    void incrementPC()   { PC++;      }
    void resetPC()       { PC = 0;    }

    uint8_t getSI() const { return SI; }

    bool stackPush(int8_t val) {
        bool ok = stack.push(val);
        if (ok) SI++;
        return ok;
    }

    bool stackPop(int8_t& out) {
        bool ok = stack.pop(out);
        if (ok) SI--;
        return ok;
    }

    int8_t getMemoryCell(int i) const { return memory.getCell(i); }
    void updateFlags(int fullResult, bool arithmeticOp) {
        flags.updateFlags(fullResult, arithmeticOp);
    }
};

// section 6
int regIndex(const string& s) {
    if (s.size() == 2 && s[0] == 'R' && s[1] >= '0' && s[1] <= '7') {
        return s[1] - '0';
    }
    return -1;
}

int parseImm(const string& s) { return stoi(s); }

string trim(const string& s) {
    int start = 0;
    int end = (int)s.size() - 1;
    while (start <= end && (s[start] == ' ' || s[start] == '\t')) start++;
    while (end >= start && (s[end] == ' ' || s[end] == '\t')) end--;
    return s.substr(start, end - start + 1);
}

string toUpper(const string& s) {
    string result = s;
    for (int i = 0; i < (int)result.size(); i++) {
        if (result[i] >= 'a' && result[i] <= 'z') {
            result[i] = result[i] - 'a' + 'A';
        }
    }
    return result;

class IOInstruction : public Instruction { // Ayra
private:
    string opcode;
    int destReg;

public:
    IOInstruction(const string& line, const string& op, int reg)
        : Instruction(line), opcode(op), destReg(reg) {}

    void execute(CPU& cpu) override {
        if (opcode == "INPUT") {
            int val;
            bool valid = false;
            while (!valid) {
                cout << "? ";
                if (!(cin >> val)) {
                    cin.clear();
                    string dummy; cin >> dummy;
                    cout << "Error: please enter an integer (-128 to 127)." << endl;
                } else {
                    valid = true;
                }
            }
            int8_t stored = (int8_t)val;
            cpu.setRegister(destReg, stored);
            cpu.getFlags().setOF(val > 127  ? 1 : 0);
            cpu.getFlags().setUF(val < -128 ? 1 : 0);
            cpu.getFlags().setZF(val == 0   ? 1 : 0);
        } else {
            cout << (int)cpu.getRegister(destReg) << endl;
        }
    }
};

class MovInstruction : public Instruction {
private:
    int destReg;
    int srcReg;
    int immVal;
    int addrReg;
    int mode;      // 0=imm, 1=reg, 2=mem-indirect

public:
    MovInstruction(const string& line, int dest, int mode, int src, int imm, int addr)
        : Instruction(line), destReg(dest), srcReg(src), immVal(imm),
          addrReg(addr), mode(mode) {}

    void execute(CPU& cpu) override {
        int result = 0;
        if (mode == 0) {
            result = immVal;
        } else if (mode == 1) {
            result = (int)cpu.getRegister(srcReg);
        } else {
            int addr = (int)(uint8_t)cpu.getRegister(addrReg);
            result = (int)cpu.readMemory(addr);
        }
        cpu.setRegister(destReg, (int8_t)result);
        cpu.updateFlags(result, false);
    }
};

class ArithmeticInstruction : public Instruction {
private:
    string opcode;
    int destReg;
    int srcReg;
    int immVal;
    int addrReg;
    int mode;       // 0=imm, 1=reg, 2=mem-indirect

public:
    ArithmeticInstruction(const string& line, const string& op,
                          int dest, int mode, int src, int imm, int addr)
        : Instruction(line), opcode(op), destReg(dest),
          srcReg(src), immVal(imm), addrReg(addr), mode(mode) {}

    void execute(CPU& cpu) override {
        int destVal = (int)cpu.getRegister(destReg);
        int srcVal  = 0;

        if (mode == 0) srcVal = immVal;
        else if (mode == 1) srcVal = (int)cpu.getRegister(srcReg);
        else {
            int addr = (int)(uint8_t)cpu.getRegister(addrReg);
            srcVal = (int)cpu.readMemory(addr);
        }

        int result = 0;
        if (opcode == "ADD") result = destVal + srcVal;
        else if (opcode == "SUB") result = destVal - srcVal;
        else if (opcode == "MUL") result = destVal * srcVal;
        else if (opcode == "DIV") {
            if (destVal == 0) {
                cout << "Error: Division by zero." << endl;
                return;
            }
            result = srcVal / destVal;
        }

        cpu.setRegister(destReg, (int8_t)result);
        cpu.updateFlags(result, true);
    }
};

class IncDecInstruction : public Instruction {
private:
    string opcode;
    int destReg;

public:
    IncDecInstruction(const string& line, const string& op, int dest)
        : Instruction(line), opcode(op), destReg(dest) {}

    void execute(CPU& cpu) override {
        int val = (int)cpu.getRegister(destReg);
        int result = (opcode == "INC") ? val + 1 : val - 1;
        cpu.setRegister(destReg, (int8_t)result);
        cpu.updateFlags(result, true);
    }
};

class ShiftInstruction : public Instruction {
private:
    string opcode;
    int destReg;
    int count;

    uint8_t toUnsigned(int8_t v) { return (uint8_t)v; }
    int8_t toSigned(uint8_t v) { return (int8_t)v; }

public:
    ShiftInstruction(const string& line, const string& op, int dest, int cnt)
        : Instruction(line), opcode(op), destReg(dest), count(cnt) {}

    void execute(CPU& cpu) override {
        uint8_t bits = toUnsigned(cpu.getRegister(destReg));
        uint8_t result = 0;
        int n = count % 8;

        if (opcode == "ROL") {
            result = (bits << n) | (bits >> (8 - n));
        } else if (opcode == "ROR") {
            result = (bits >> n) | (bits << (8 - n));
        } else if (opcode == "SHL") {
            result = (n >= 8) ? 0 : (bits << n);
        } else if (opcode == "SHR") {
            result = (n >= 8) ? 0 : (bits >> n);
        }

        int8_t signed_result = toSigned(result);
        cpu.setRegister(destReg, signed_result);
        cpu.updateFlags((int)signed_result, false);
    }
};

class LoadInstruction : public Instruction {
private:
    int destReg;
    int directAddr;
    int addrReg;
    int mode;       // 0=direct, 1=reg-indirect

public:
    LoadInstruction(const string& line, int dest, int mode, int addr, int reg)
        : Instruction(line), destReg(dest), directAddr(addr),
          addrReg(reg), mode(mode) {}

    void execute(CPU& cpu) override {
        int addr = 0;
        if (mode == 0) {
            addr = directAddr;
        } else {
            addr = (int)(uint8_t)cpu.getRegister(addrReg);
        }
        int8_t val = cpu.readMemory(addr);
        cpu.setRegister(destReg, val);
        cpu.updateFlags((int)val, false);
    }
};

class StoreInstruction : public Instruction {
private:
    int srcReg;
    int directAddr;
    int addrReg;
    int mode;        // 0=direct, 1=reg-indirect

public:
    StoreInstruction(const string& line, int src, int mode, int addr, int areg)
        : Instruction(line), srcReg(src), directAddr(addr),
          addrReg(areg), mode(mode) {}

    void execute(CPU& cpu) override {
        int addr = 0;
        if (mode == 0) {
            addr = directAddr;
        } else {
            addr = (int)(uint8_t)cpu.getRegister(addrReg);
        }
        cpu.writeMemory(addr, cpu.getRegister(srcReg));
    }
};

class FlagResetInstruction : public Instruction {
private:
    string flagName;

public:
    FlagResetInstruction(const string& line, const string& flag)
        : Instruction(line), flagName(flag) {}

    void execute(CPU& cpu) override {
        if      (flagName == "CF") cpu.getFlags().setCF(0);
        else if (flagName == "OF") cpu.getFlags().setOF(0);
        else if (flagName == "UF") cpu.getFlags().setUF(0);
        else if (flagName == "ZF") cpu.getFlags().setZF(0);
    }
};

class StackInstruction : public Instruction {
private:
    string opcode;
    int reg;

public:
    StackInstruction(const string& line, const string& op, int r)
        : Instruction(line), opcode(op), reg(r) {}

    void execute(CPU& cpu) override {
        if (opcode == "PUSH") {
            cpu.stackPush(cpu.getRegister(reg));
        } else {
            int8_t val;
            bool ok = cpu.stackPop(val);
            if (!ok) {
                cout << "Fatal Error: Stack underflow." << endl;
                exit(1);
            }
            cpu.setRegister(reg, val);
            cpu.updateFlags((int)val, false);
        }
    }
};
}

// =============================================================================
// SECTION 7 - RUNNER CLASS
// =============================================================================
class Runner {
private:
    CPU cpu;                 
    MyList instructionLines; 
    MyQueue instructionQueue;

    Instruction* parseIO(const string& raw, const string& op, const string& rest) {
        int r = regIndex(toUpper(trim(rest)));
        if (r < 0) return nullptr;
        return new IOInstruction(raw, op, r);
    }

    Instruction* parseMov(const string& raw, const string& rest) {
        int comma = rest.find(',');
        if (comma == (int)string::npos) return nullptr;
        string destStr = toUpper(trim(rest.substr(0, comma)));
        string srcStr  = trim(rest.substr(comma + 1));
        int dest = regIndex(destStr);
        if (dest < 0) return nullptr;

        if (srcStr[0] == '[' && srcStr.back() == ']') {
            string in = toUpper(trim(srcStr.substr(1, srcStr.size() - 2)));
            int addr = regIndex(in);
            if (addr < 0) return nullptr;
            return new MovInstruction(raw, dest, 2, -1, 0, addr);
        }
        int src = regIndex(toUpper(srcStr));
        if (src >= 0) return new MovInstruction(raw, dest, 1, src, 0, -1);
        return new MovInstruction(raw, dest, 0, -1, parseImm(srcStr), -1);
    }

    Instruction* parseArithmetic(const string& raw, const string& op, const string& rest) {
        int comma = rest.find(',');
        if (comma == (int)string::npos) return nullptr;
        int dest = regIndex(toUpper(trim(rest.substr(0, comma))));
        string srcStr = trim(rest.substr(comma + 1));
        if (dest < 0) return nullptr;

        if (srcStr[0] == '[' && srcStr.back() == ']') {
            string in = toUpper(trim(srcStr.substr(1, srcStr.size() - 2)));
            int addr = regIndex(in);
            if (addr < 0) return nullptr;
            return new ArithmeticInstruction(raw, op, dest, 2, -1, 0, addr);
        }
        int src = regIndex(toUpper(srcStr));
        if (src >= 0) return new ArithmeticInstruction(raw, op, dest, 1, src, 0, -1);
        return new ArithmeticInstruction(raw, op, dest, 0, -1, parseImm(srcStr), -1);
    }

    Instruction* parseLoad(const string& raw, const string& rest) {
        int comma = rest.find(',');
        if (comma == (int)string::npos) return nullptr;
        int dest = regIndex(toUpper(trim(rest.substr(0, comma))));
        string srcStr = trim(rest.substr(comma + 1));
        if (dest < 0) return nullptr;

        if (srcStr[0] == '[' && srcStr.back() == ']') {
            string in = toUpper(trim(srcStr.substr(1, srcStr.size() - 2)));
            int src = regIndex(in);
            if (src >= 0) return new LoadInstruction(raw, dest, 1, 0, src);
            return new LoadInstruction(raw, dest, 0, parseImm(in), -1);
        }
        return nullptr;
    }

    Instruction* parseStore(const string& raw, const string& rest) {
        int comma = rest.find(',');
        if (comma == (int)string::npos) return nullptr;
        string first = trim(rest.substr(0, comma));
        string second = trim(rest.substr(comma + 1));

        if (first[0] == '[' && first.back() == ']') {
            int addrReg = regIndex(toUpper(trim(first.substr(1, first.size() - 2))));
            int srcReg = regIndex(toUpper(second));
            if (addrReg < 0 || srcReg < 0) return nullptr;
            return new StoreInstruction(raw, srcReg, 1, 0, addrReg);
        }
        int srcReg = regIndex(toUpper(first));
        if (srcReg >= 0) return new StoreInstruction(raw, srcReg, 0, parseImm(second), -1);
        return nullptr;
    }

    Instruction* parseIncDec(const string& raw, const string& op, const string& rest) {
        int r = regIndex(toUpper(trim(rest)));
        if (r < 0) return nullptr;
        return new IncDecInstruction(raw, op, r);
    }

    Instruction* parseShift(const string& raw, const string& op, const string& rest) {
        int comma = rest.find(',');
        if (comma == (int)string::npos) return nullptr;
        int r = regIndex(toUpper(trim(rest.substr(0, comma))));
        int cnt = parseImm(trim(rest.substr(comma + 1)));
        if (r < 0) return nullptr;
        return new ShiftInstruction(raw, op, r, cnt);
    }

    Instruction* parseStack(const string& raw, const string& op, const string& rest) {
        int r = regIndex(toUpper(trim(rest)));
        if (r < 0) return nullptr;
        return new StackInstruction(raw, op, r);
    }

    Instruction* parseReset(const string& raw, const string& rest) {
        string flag = toUpper(trim(rest));
        if (flag=="CF" || flag=="OF" || flag=="UF" || flag=="ZF")
            return new FlagResetInstruction(raw, flag);
        return nullptr;
    }

    Instruction* parseInstruction(const string& rawLine) {
        string line = trim(rawLine);
        if (line.empty()) return nullptr;

        string opcode = ""; int i = 0;
        while (i < (int)line.size() && line[i] != ' ' && line[i] != '\t') opcode += line[i++];
        opcode = toUpper(opcode);
        string rest = (i < (int)line.size()) ? trim(line.substr(i)) : "";

        if (opcode == "MOV") return parseMov(rawLine, rest);
        if (opcode == "LOAD") return parseLoad(rawLine, rest);
        if (opcode == "STORE") return parseStore(rawLine, rest);
        if (opcode == "ADD" || opcode == "SUB" || opcode == "MUL" || opcode == "DIV")
            return parseArithmetic(rawLine, opcode, rest);
        if (opcode == "INPUT" || opcode == "DISPLAY") return parseIO(rawLine, opcode, rest);
        if (opcode == "INC" || opcode == "DEC") return parseIncDec(rawLine, opcode, rest);
        if (opcode == "ROL" || opcode == "ROR" || opcode == "SHL" || opcode == "SHR")
            return parseShift(rawLine, opcode, rest);
        if (opcode == "PUSH" || opcode == "POP") return parseStack(rawLine, opcode, rest);
        if (opcode == "RESET") return parseReset(rawLine, rest);

        cout << "Error: Unknown instruction '" << opcode << "'" << endl;
        return nullptr;
    }

public:
    bool loadProgram(const string& filename) {
        ifstream file(filename.c_str());
        if (!file.is_open()) {
            cout << "Error: Cannot open file '" << filename << "'" << endl;
            return false;
        }

        string line;
        while (getline(file, line)) {
            line = trim(line);
            if (line.empty()) continue; 
            instructionLines.push_back(line);
            instructionQueue.enqueue(line); 
        }
        file.close();
        return true;
    }

    void run() {
        cpu.resetPC();
        int count = instructionQueue.getSize();
        Instruction** program = new Instruction*[count];
        int loaded = 0;

        while (!instructionQueue.isEmpty()) {
            string raw = instructionQueue.dequeue();
            program[loaded] = parseInstruction(raw);
            loaded++;
        }

        cout << "\n=== Start program ===" << endl;
        cout << "R0  R1  R2  R3  R4  R5  R6  R7  PC" << endl;
        cout << "  0   0   0   0   0   0   0   0   " << (int)cpu.getPC() << endl;

        for (int idx = 0; idx < loaded; idx++) {
            if (program[idx] == nullptr) {
                cpu.incrementPC();
                continue;
            }

            cout << "\n> " << instructionLines.get(idx) << endl;
            program[idx]->execute(cpu);
            cpu.incrementPC();

            cout << "R0=" << setw(3) << (int)cpu.getRegister(0)
                 << "  R1=" << setw(3) << (int)cpu.getRegister(1)
                 << "  R2=" << setw(3) << (int)cpu.getRegister(2)
                 << "  R3=" << setw(3) << (int)cpu.getRegister(3)
                 << "  R4=" << setw(3) << (int)cpu.getRegister(4)
                 << "  R5=" << setw(3) << (int)cpu.getRegister(5)
                 << "  R6=" << setw(3) << (int)cpu.getRegister(6)
                 << "  R7=" << setw(3) << (int)cpu.getRegister(7)
                 << "  PC=" << (int)cpu.getPC() << endl;
        }

        cout << "\n=== End of program ===" << endl;
        for (int i = 0; i < loaded; i++) delete program[i];
        delete[] program;
    }

    void dumpToScreen() const {
        cout << "\n=== Virtual Machine State Dump ===" << endl;
        cout << "Registers:" << endl;
        for (int i = 0; i < 8; i++) cout << "  R" << i << " = " << (int)cpu.getRegister(i) << endl;

        cout << "Flags:" << endl;
        cout << "  OF=" << cpu.getFlags().getOF()
             << "  UF=" << cpu.getFlags().getUF()
             << "  CF=" << cpu.getFlags().getCF()
             << "  ZF=" << cpu.getFlags().getZF() << endl;

        cout << "PC = " << (int)cpu.getPC() << endl;
        cout << "SI = " << (int)cpu.getSI() << endl;

        cout << "Memory (0..63):" << endl;
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                int addr = row * 8 + col;
                cout << setw(5) << (int)(uint8_t)cpu.getMemoryCell(addr);
            }
            cout << endl;
        }
    }

    void dumpToFile(const string& filename) const {
        ofstream out(filename.c_str());
        if (!out.is_open()) return;

        out << "#Begin#" << endl;
        out << "#Registers#";
        for (int i = 0; i < 8; i++) {
            int v = (int)(uint8_t)cpu.getRegister(i);
            out << setw(4) << setfill('0') << v << "#";
        }
        out << endl;

        out << "#Flags#"
            << "OF#" << cpu.getFlags().getOF() << "#"
            << "UF#" << cpu.getFlags().getUF() << "#"
            << "CF#" << cpu.getFlags().getCF() << "#"
            << "ZF#" << cpu.getFlags().getZF() << "#" << endl;

        out << "#PC#" << setw(4) << setfill('0') << (int)cpu.getPC() << "#" << endl;

        out << "#Memory#" << endl;
        for (int row = 0; row < 8; row++) {
            out << "#";
            for (int col = 0; col < 8; col++) {
                int addr = row * 8 + col;
                int v = (int)(uint8_t)cpu.getMemoryCell(addr);
                out << setw(4) << setfill('0') << v << "#";
            }
            out << endl;
        }
        out << "#End#" << endl;
        out.close();
        cout << "\nOutput written to '" << filename << "'" << endl;
    }
};

// =============================================================================
// SECTION 8 - MAIN FUNCTION
// =============================================================================
int main(int argc, char* argv[]) {
    cout << "==================================================" << endl;
    cout << "  CCP6124 Virtual Machine Interpreter - TT4L_G01" << endl;
    cout << "==================================================" << endl;

    string inputFile;
    if (argc >= 2) {
        inputFile = argv[1];
    } else {
        cout << "Usage: " << argv[0] << " <program.asm>" << endl;
        cout << "Enter .asm filename: ";
        cin >> inputFile;
    }

    string outputFile = inputFile;
    int dot = (int)outputFile.rfind('.');
    if (dot != (int)string::npos) outputFile = outputFile.substr(0, dot);
    outputFile += "_output.txt";

    Runner runner;
    if (!runner.loadProgram(inputFile)) return 1;

    runner.run();
    runner.dumpToScreen();
    runner.dumpToFile(outputFile);

    return 0;
}