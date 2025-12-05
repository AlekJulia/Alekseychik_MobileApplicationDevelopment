module uart_echo #(
    parameter CLK_FREQ = 50_000_000,
    parameter BAUD_RATE = 115200
)(
    input  logic clk,
    input  logic reset,
    input  logic uart_rx,
    output logic uart_tx
);

    // сигналы приёмника
    logic [7:0] received_data;
    logic data_valid;
    logic rx_error;
    
    // сигналы передатчика
    logic tx_busy;
    logic tx_done;
    
    // управление передачей
    logic start_transmission;
    
    // ДОБАВЛЕНО: управление потоком
    logic allow_transmit;
    logic is_control_char;
    
    // ДОБАВЛЕНО: шифрование
    logic [7:0] encrypted_data;
    logic [7:0] tx_data_final;
    
    // РЕГИСТРЫ для синхронизации
    logic is_control_char_reg;      // зарегистрированный is_control_char
    logic is_control_char_prev;     // предыдущее значение
    logic data_valid_prev;          // предыдущий data_valid
    logic [7:0] received_data_reg;  // зарегистрированные данные
    logic data_valid_reg;           // зарегистрированный флаг
  
    // ПРИЁМНИК
    uart_receiver #(
        .CLK_FREQ(CLK_FREQ),
        .BAUD_RATE(BAUD_RATE)
    ) receiver (
        .clk(clk),
        .reset(reset),     
        .rx_in(uart_rx),
        .rx_data(received_data),
        .rx_valid(data_valid),
        .rx_error(rx_error)
    );
    
    // ДОБАВЛЕНО: КОНТРОЛЬ ПОТОКА
    flow_control flow_ctrl (
        .clk(clk),
        .reset(reset), 
        .rx_data(received_data),
        .rx_valid(data_valid),
        .allow_transmit(allow_transmit),
        .is_control_char(is_control_char)  // КОМБИНАЦИОННЫЙ!
    );
    
    // СИНХРОНИЗАЦИЯ: задержка всех сигналов на 1 такт
    always_ff @(posedge clk) begin
        if (!reset) begin
            received_data_reg <= 8'h00;
            data_valid_reg <= 1'b0;
            is_control_char_reg <= 1'b0;
            is_control_char_prev <= 1'b0;
            data_valid_prev <= 1'b0;
        end else begin
            // Основные регистры
            received_data_reg <= received_data;
            data_valid_reg <= data_valid;
            is_control_char_reg <= is_control_char;
            
            // Предыдущие значения для обнаружения фронтов
            is_control_char_prev <= is_control_char_reg;
            data_valid_prev <= data_valid_reg;
        end
    end
    
    // ДОБАВЛЕНО: ШИФРАТОР - получает ЗАРЕГИСТРИРОВАННЫЕ данные!
    uart_caesar_cipher cipher (
        .plain_data(received_data_reg),
        .is_control_char(is_control_char_reg),  // зарегистрированный флаг!
        .cipher_data(encrypted_data)
    );
    
    // Данные для передачи
    assign tx_data_final = encrypted_data;
    
    // ОСНОВНАЯ ЛОГИКА - КОМБИНАЦИЯ ОБОИХ ПРОВЕРОК!
    always_ff @(posedge clk) begin
        if (!reset) begin         
            start_transmission <= 1'b0;
        end else begin            
            // По умолчанию не запускаем передачу
            start_transmission <= 1'b0;
            
            // Запускаем передачу ТОЛЬКО если ВСЕ условия выполнены:
            // 1. Пришли данные (data_valid_reg) - ЗАРЕГИСТРИРОВАННЫЙ
            // 2. Это фронт данных (!data_valid_prev)
            // 3. Передатчик свободен (!tx_busy)
            // 4. Разрешена передача (allow_transmit)
            // 5. ПРЕДЫДУЩИЙ символ не был управляющим (!is_control_char_prev) ← из КОДА 1
            // 6. ТЕКУЩИЙ символ не управляющий (!is_control_char_reg) ← из КОДА 1
            
            if (data_valid_reg && !data_valid_prev && !tx_busy && 
                allow_transmit && !is_control_char_prev && !is_control_char_reg) begin
                start_transmission <= 1'b1;
            end
        end
    end
    
    // ПЕРЕДАТЧИК
    uart_transmitter #(
        .CLK_FREQ(CLK_FREQ),
        .BAUD_RATE(BAUD_RATE)
    ) transmitter (
        .clk(clk),
        .reset(reset),      
        .tx_start(start_transmission),
        .tx_data(tx_data_final),
        .tx_out(uart_tx),
        .tx_busy(tx_busy),
        .tx_done(tx_done)
    );
     
endmodule