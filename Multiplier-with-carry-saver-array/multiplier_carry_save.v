module multiplier(M, Q, S);
    parameter n = 8; // Number of bits of multiplier operand
    input [n - 1:0] M, Q; // Multiplicand & Multiplier
    output reg [2 * n - 1:0] S; // Multiplication Result
    reg [n - 1:0] Carry [2:n]; // 2D bit array to store carry bits
    reg [n - 1:0] Sum [2:n]; // 2D bit array to store each level of sum bit

    integer i, row, count;
    reg a1, a2, a3, carryout;
    
    always @* begin
        for (i = 0; i < n; i = i + 1) begin
            Carry[2][i] = M[i] & Q[1];
        end
    
        S[0] = M[0] * Q[0];
        count = 1;
        Sum[2][n - 1] = 0;
        for (i = 0; i < n - 1; i = i + 1) begin
            Sum[2][i] = M[i] * Q[0];
        end
        
        for (row = 2; row < n; row = row + 1) begin            
            for (i = 0; i < n; i = i + 1) begin
                if (i == 0) begin
                    S[count] = Carry[row][i] ^ Sum[row][i];
                    Carry[row + 1][i] = Carry[row][i] & Sum[row][i];
                    count = count + 1;
                end
                else begin
                    Sum[row + 1][i - 1] = Carry[row][i] ^ Sum[row][i] ^ (M[i - 1] & Q[row]);
                    a1 = Carry[row][i] & Sum[row][i];
                    a2 = Sum[row][i] & M[i - 1] & Q[row];
                    a3 = M[i - 1] & Q[row] & Carry[row][i];
                    Carry[row + 1][i] = a1 | a2 | a3;
                end
            end
            Sum[row + 1][n - 1] = M[n - 1] & Q[row];
        end

        // The last level is a ripple carry adder
        carryout = 0;
        for (i = 0; i < n; i = i + 1) begin
            S[count] = Carry[n][i] ^ Sum[n][i] ^ carryout;
            a1 = Carry[n][i] & Sum[n][i];
            a2 = Carry[n][i] & carryout;
            a3 = Sum[n][i] & carryout;
            carryout = a1 | a2 | a3;
	    count = count + 1;
        end
        S[2 * n - 1] = carryout;
    end
endmodule