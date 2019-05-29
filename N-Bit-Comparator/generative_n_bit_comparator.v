module fulladder(carryin, x, y, s, carryout);
    input carryin, x, y;
    output s, carryout;
    
    assign s = carryin ^ x ^ y;
    assign carryout = (carryin & x) | (carryin & y) | (x & y);
endmodule

module n_bit_comparator(X, Y, S, Z, N, V, Equal, Smaller, Larger);
    parameter n = 32; //Number of bits; MSB for sign
    input [n - 1:0] X, Y;
    output [n - 1:0] S;
    output Z, N, V, Equal, Smaller, Larger;
    wire [n:0] C;
    genvar i;
    
    assign C[0] = 1; // Carry in for level 0 digit
    generate
        for (i = 0; i < n; i = i + 1) begin: adder
            fulladder stage(C[i], X[i], ~Y[i], S[i], C[i + 1]);
        end
    endgenerate

    assign Z = !S; // Difference equal to 0
    assign N = S[n - 1]; // Output is negative
    assign V = C[n] ^ C[n - 1]; // Overflow occurs

    assign Equal = Z;
    assign Smaller = N ^ V;
    assign Larger = ~(Z + (N ^ V));

endmodule
