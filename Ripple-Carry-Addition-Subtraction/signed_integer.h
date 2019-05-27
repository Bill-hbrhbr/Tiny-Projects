#ifndef SIGNED_INTEGER_H
#define SIGNED_INTEGER_H
#include <iostream>
#include <vector>
#include <iterator>
#include "ripple_carry_adder.h"

using std::vector;
constexpr extern int NUM_OF_BITS = 16;

vector<bool> convertDecimalToBinary(int value, int numOfBits = NUM_OF_BITS);
int convertBinaryToDecimal(vector<bool> vec, int numOfBits = NUM_OF_BITS);

class SignedInteger {
friend SignedInteger SignedIntegerOperation(SignedInteger x, SignedInteger y, char op);
friend SignedInteger operator +(SignedInteger x, SignedInteger y);
friend SignedInteger operator -(SignedInteger x, SignedInteger y);
public:
    //Constructors
    SignedInteger(): decimalNum(0), bitVec(numOfBits, 0) {}
    SignedInteger(int value);
    SignedInteger(const vector<bool> &vec);
    
    //Accessors
    int getDecimalNumber() {return decimalNum;}
    vector<bool> getBinaryVector() {return bitVec;}
    int getNumOfBits() {return numOfBits;}
    
    //Output functions
    void printDecimalNumber() const;
    void printBinaryNumber() const;
    
private:
    static constexpr int numOfBits = NUM_OF_BITS;
    static RippleCarryAdder rca;
    int decimalNum;
    vector<bool> bitVec;
    
};

RippleCarryAdder rca;

//Friend function declarations
SignedInteger SignedIntegerOperation(SignedInteger x, SignedInteger y, char op);
SignedInteger operator +(SignedInteger x, SignedInteger y);
SignedInteger operator -(SignedInteger x, SignedInteger y);

//Non-default constructors' definitions
SignedInteger::SignedInteger(int value): 
                             decimalNum(value), bitVec(convertDecimalToBinary(value, numOfBits)) {}
    
SignedInteger::SignedInteger(const vector<bool> &vec) {
    bitVec.clear();
    int size = vec.size();
    for (int i = 0; i < numOfBits && i < size; ++i) {
        bitVec.push_back(vec[i]);
    }
    decimalNum = convertBinaryToDecimal(bitVec, numOfBits);
}

//Print functions
void SignedInteger::printDecimalNumber() const {
    std::cout << "The decimal representation: " << decimalNum << std::endl;
}

void SignedInteger::printBinaryNumber() const {
    std::cout << "The binary representation: ";
    //Reverse the bitVec for correct binary representation starting from the most significant bit
    for (auto iter = bitVec.crbegin(); iter != bitVec.crend(); ++iter) {
        std::cout << *iter;
    }
    std::cout << std::endl;
}

//Binary/Decimal conversion functions
vector<bool> convertDecimalToBinary(int value, int numOfBits) {
    bool isNegative;
    vector<bool> bitVec;
    if (value >= 0) {
        isNegative = false;
    } else {
        value = -value;
        isNegative = true;
    }
    //Every bit except for the sign bit
    for (int i = 1; i <= numOfBits - 1; ++i) {
        bitVec.push_back(value % 2);
        value /= 2;
    }
    if (isNegative && value == 1) {
        bitVec.push_back(1);
        return bitVec;
    } else {
        bitVec.push_back(0);
    }
    if (isNegative) {
        int i = 0;
        while (i < numOfBits && bitVec[i] == 0) {
            ++i;
        }
        ++i;
        while (i < numOfBits) {
            bitVec[i] = !bitVec[i];
            ++i;
        }
    }
    return bitVec;
}

int convertBinaryToDecimal(vector<bool> vec, int numOfBits) {
    bool isNegative = vec[numOfBits - 1];
    if (isNegative) {
        int i = 0;
        while (i < numOfBits && vec[i] == 0) {
            ++i;
        }
        ++i;
        while (i < numOfBits) {
            vec[i] = !vec[i];
            ++i;
        }
    }
    if (vec[numOfBits - 1] == 1) {
        int result = -1;
        for (int i = 1; i < numOfBits; ++i) {
            result *= 2;
        }
        return result;
    }
    
    int result = 0, multiplier = 1;
    for (auto iter = vec.cbegin(); iter != vec.cend(); ++iter) {
        result += *iter * multiplier;
        multiplier *= 2;
    }
    return isNegative ? -result : result;
}

//Friend Functions: Operations
SignedInteger SignedIntegerOperation(SignedInteger x, SignedInteger y, char op) {
    switch (op) {
        case '+': return x + y;
        case '-': return x - y;
        default:
            std::cout << "*Invalid Operator*" << std::endl;
    }
    return SignedInteger();
}

//Operator Overloading for + and -
SignedInteger operator +(SignedInteger x, SignedInteger y) {
    bool hasOverflow;
    SignedInteger result(rca.add(x.bitVec, y.bitVec, hasOverflow, 0));
    if (hasOverflow) {
        std::cout << "Integer Overflow (" << SignedInteger::numOfBits << "bits)" << std::endl;
        std::cout << "Addition Result Undefined" << std::endl;
    }
    return result;
}

SignedInteger operator -(SignedInteger x, SignedInteger y) {
    bool hasOverflow;
    SignedInteger result(rca.add(x.bitVec, y.bitVec, hasOverflow, 1));
    if (hasOverflow) {
        std::cout << "Integer Overflow (" << SignedInteger::numOfBits << "bits)" << std::endl;
        std::cout << "Subtraction Result Undefined" << std::endl;
    }
    return result;
}

#endif