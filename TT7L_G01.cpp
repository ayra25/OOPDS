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