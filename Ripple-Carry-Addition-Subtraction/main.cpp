#include <iostream>
#include <vector>
#include "signed_integer.h"

int main(void)
{
    SignedInteger dummy;
    int _x, _y;
    std::cout << "Enter two numbers (" << dummy.getNumOfBits() << "bits): ";
    std::cin >> _x >> _y;
    SignedInteger x(_x), y(_y);
    char op;
    std::cout << "Enter the operation (+ or -): ";
    std::cin >> std::ws >> op;
    std::cout << std::endl;
    
    SignedInteger z = SignedIntegerOperation(x, y, op);
    std::cout << "Arithmetic Operation Result:" << std::endl;
    z.printDecimalNumber();
    z.printBinaryNumber();
	return 0;
}
