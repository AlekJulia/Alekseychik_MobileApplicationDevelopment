module uart_caesar_cipher (
    input  logic [7:0] plain_data,
    input  logic is_control_char,
    output logic [7:0] cipher_data
);
    
    always_comb begin
        if (is_control_char) begin
            cipher_data = 8'h00;  // ВСЕГДА 0x00 для Ctrl+S/Ctrl+Q
        end else if (plain_data >= 8'h41 && plain_data <= 8'h5A) begin
            cipher_data = (plain_data <= 8'h5A - 3) ? plain_data + 3 : plain_data - 23;
        end else if (plain_data >= 8'h61 && plain_data <= 8'h7A) begin
            cipher_data = (plain_data <= 8'h7A - 3) ? plain_data + 3 : plain_data - 23;
        end else begin
            cipher_data = plain_data;
        end
    end
endmodule