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