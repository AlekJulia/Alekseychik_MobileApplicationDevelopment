`define v1_2 "-name IO_STANDARD \"1.2-V\""
`define v2_5 "-name IO_STANDARD \"2.5-V\""

`define pin_clk      "H12"
`define pin_reset    "P11"
`define pin_uart_rx  "M9"
`define pin_uart_tx  "L9"

module top_uart_caesar(
    (* chip_pin = `pin_clk *)      input  clk,
    (* altera_attribute = `v1_2, chip_pin = `pin_reset *)   input  reset_n, // активный низкий (кнопка на 1.2V)
    (* altera_attribute = `v2_5, chip_pin = `pin_uart_rx *) input  UARTrx,
    (* altera_attribute = `v2_5, chip_pin = `pin_uart_tx *) output UARTtx
);

    // Подключаем uart_caesar (reset_n — active low)
    uart_caesar uart_caesar_inst (
        .clk50(clk),
        .reset_n(reset_n),
        .uart_rx(UARTrx),
        .uart_tx(UARTtx)
    );

endmodule