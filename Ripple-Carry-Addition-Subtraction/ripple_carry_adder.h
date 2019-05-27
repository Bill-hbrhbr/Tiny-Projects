#ifndef RIPPLE_CARRY_ADDER_H
#define RIPPLE_CARRY_ADDER_H
#include <vector>
using std::vector;

class FullAdder {
public:
    FullAdder() = default;
    bool add(bool x, bool y, bool &carry) {
        int s = x ^ y ^ carry;
        carry = (x & y) | (x & carry) | (y & carry);
        return s;
    }
};


class RippleCarryAdder {
public:
    RippleCarryAdder() = default;
    vector<bool> add(vector<bool> &x, vector<bool> &y, bool carryIn = 0);
private:
    FullAdder fa;
};

vector<bool> RippleCarryAdder::add(vector<bool> &x, vector<bool> &y, bool carryIn) {
    int size = x.size();
    bool control = carryIn;
    vector<bool> result;
    for (int i = 0; i < size; ++i) {
        result.push_back(fa.add(x[i], y[i] ^ control, carryIn));
    }
    return result;
}

#endif