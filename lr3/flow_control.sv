module flow_control (
    input  logic clk,
    input  logic reset, 
    input  logic [7:0] rx_data,
    input  logic rx_valid,
    output logic allow_transmit,
    output logic is_control_char
);

    logic allow_reg;
    
    always_ff @(posedge clk or negedge reset) begin
        if (!reset) begin
            allow_reg <= 1'b1;
        end else if (rx_valid) begin
            case (rx_data)
                8'h11: allow_reg <= 1'b1;  // XON
                8'h13: allow_reg <= 1'b0;  // XOFF
            endcase
        end 
    end 
    
    // Комбинационный - в МОМЕНТ приема
    assign is_control_char = rx_valid && (rx_data == 8'h11 || rx_data == 8'h13);
    assign allow_transmit = allow_reg;

endmodule