// uart.sv — эхо-контроллер - все что приходит на вход rx автоматические отправляется обратно на выход tx
module uart(
    input  logic clk50,
    input  logic reset_n,   // active-low
    input  logic uart_rx,
    output logic uart_tx
);

    // Формируем active-high reset (через assign — нельзя инициализировать выражением при объявлении)
    logic reset;
    assign reset = ~reset_n;

    logic [7:0] rx_byte; // 8 бит данных
    logic rx_ready; // флаг что приёмник получил новый байт

    logic tx_busy; // флаг что передатчик занят передачей
    logic tx_start; // уровень запроса на передачу 
    logic tx_start_pulse; // импульс запуска передатчика 
    logic rx_ready_d; // задержанная версия rx_ready для детектирования фронта

    // --- RX instance ---
    uartrx #(.CLK_FREQ(50_000_000), .BAUD(6_250_000)) RX (
        .clk(clk50),
        .reset(reset),
        .rx(uart_rx), // последовательный ввод данных 
        .data_valid(rx_ready), // вывод флаг данные готовы
        .data(rx_byte) // вывод буфера принятых ланных
    );

    // --- TX instance ---
    // We will send a single-cycle start pulse to TX when a new byte is ready and TX is free
    uarttx #(.CLK_FREQ(50_000_000), .BAUD(6_250_000)) TX (
        .clk(clk50),
        .reset(reset),
        .start(tx_start_pulse),
        .data(rx_byte), // ввод полученных данных от rx
        .tx(uart_tx), // последовательный вывод данных
        .busy(tx_busy) // вывод флага занятости
    );

    // --- generate single-cycle pulse from rx_ready (strobe) ---
    always_ff @(posedge clk50 or posedge reset) begin
        if (reset) begin
            rx_ready_d      <= 1'b0;
            tx_start        <= 1'b0;
            tx_start_pulse  <= 1'b0;
        end else begin
            rx_ready_d <= rx_ready;

            // хотим передать когда ЕСТЬ данные И передатчик СВОБОДЕН
            tx_start <= (rx_ready && !tx_busy);

            // импульс запуска tx
            tx_start_pulse <= (rx_ready && !rx_ready_d && !tx_busy); // защита от многократного запуска
        end
    end

endmodule
