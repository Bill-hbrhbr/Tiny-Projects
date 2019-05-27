#include <iostream>
#include <vector>
#include "signed_integer.h"

int main(void)
{
    int _x, _y;
    std::cout << "Enter two numbers: ";
    std::cin >> _x >> _y;
    SignedInteger x(_x), y(_y);
    char op;
    std::cout << "Enter your operation: ";
    std::cin >> std::ws >> op;
    SignedInteger z = SignedIntegerOperation(x, y, op);
    z.printDecimalNumber();
    z.printBinaryNumber();
	return 0;
}
