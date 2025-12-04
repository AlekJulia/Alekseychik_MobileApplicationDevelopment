// ------------------------------------------------------------
// UART + Caesar cipher
// Принимает байт → кодирует Цезарем → отправляет обратно
// ------------------------------------------------------------
module uart_caesar #(
    parameter CLK_FREQ = 50_000_000,
	 parameter BAUD     = 115_200,
    //parameter BAUD     = 6_250_000,
    parameter SHIFT    = 3
)(
    input  logic clk50,
    input  logic reset_n,   // active-low
    input  logic uart_rx,
    output logic uart_tx
);

    // Формируем active-high reset (через assign — нельзя инициализировать выражением при объявлении)
    logic reset;
    assign reset = ~reset_n;

    // Byte from RX
    logic [7:0] rx_byte;
    logic       rx_ready;

    // Encoded byte
    logic [7:0] enc_byte;

    // TX control
    logic tx_busy;
    logic tx_start;
    logic tx_start_pulse;
    logic rx_ready_d;

    // ---------------------------
    // RX instance
    // ---------------------------
    uartrx #(.CLK_FREQ(CLK_FREQ), .BAUD(BAUD)) RX (
        .clk(clk50),
        .reset(reset),
        .rx(uart_rx),
        .data_valid(rx_ready),
        .data(rx_byte)
    );

    // ---------------------------
    // Caesar cipher module
    // ---------------------------
    caesar_cipher #(.SHIFT(SHIFT)) C1 (
        .in_char(rx_byte),
        .out_char(enc_byte)
    );

    // ---------------------------
    // TX instance
    // ---------------------------
    uarttx #(.CLK_FREQ(CLK_FREQ), .BAUD(BAUD)) TX (
        .clk(clk50),
        .reset(reset),
        .start(tx_start_pulse),
        .data(enc_byte),
        .tx(uart_tx),
        .busy(tx_busy)
    );

    // ---------------------------
    // Generate TX start pulse
    // ---------------------------
    always_ff @(posedge clk50 or posedge reset) begin
        if (reset) begin
            rx_ready_d      <= 1'b0;
            tx_start        <= 1'b0;
            tx_start_pulse  <= 1'b0;
        end else begin
            rx_ready_d <= rx_ready;

            // передаём только когда есть данные и TX свободен
            tx_start <= rx_ready && !tx_busy;

            // однократный импульс
            tx_start_pulse <= (rx_ready && !rx_ready_d && !tx_busy);
        end
    end

endmodule
