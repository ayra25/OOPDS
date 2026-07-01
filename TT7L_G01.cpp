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
}