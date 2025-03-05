module ssdec(
  input logic [3:0] in,
  input logic enable,
  output logic [6:0] out
);

  // Direct assignment with conditional logic
  assign out = (enable) ? 
    (in == 4'b0000) ? 7'b0111111 :  // none
    (in == 4'b0001) ? 7'b0000110 :  // one
    (in == 4'b0010) ? 7'b1011011 :  // two
    (in == 4'b0011) ? 7'b1001111 :  // three
    (in == 4'b0100) ? 7'b1100110 :  // four
    (in == 4'b0101) ? 7'b1101101 :  // five
    (in == 4'b0110) ? 7'b1111101 :  // six
    (in == 4'b0111) ? 7'b0000111 :  // seven
    (in == 4'b1000) ? 7'b1111111 :  // eight
    (in == 4'b1001) ? 7'b1100111 :  // nine
    (in == 4'b1010) ? 7'b1110111 :  // A
    (in == 4'b1011) ? 7'b1111100 :  // b
    (in == 4'b1100) ? 7'b0111001 :  // C
    (in == 4'b1101) ? 7'b1011110 :  // d
    (in == 4'b1110) ? 7'b1111001 :  // E
    (in == 4'b1111) ? 7'b1110001 :  // F
    7'b0000000 : 
    7'b0000000;  
endmodule