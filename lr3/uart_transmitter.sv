// передатчик
module uart_transmitter #(
    parameter CLK_FREQ = 50_000_000, // 50 МГц
    parameter BAUD_RATE = 115200
)(
    input  logic clk, // тактовый сигнал
    input  logic reset, // сброс
    input  logic tx_start, // флаг начала передачи
    input  logic [7:0] tx_data, // данные для передачи
    output logic tx_out, // выходной сигнал
    output logic tx_busy, // флаг занятости линии (передаёт)
    output logic tx_done // флаг завершения передачи
);

    localparam integer TICKS_PER_BIT = CLK_FREQ / BAUD_RATE; // сколько тактов на бит
    localparam integer CNT_WIDTH = $clog2(TICKS_PER_BIT); // счётчик для подсчёта тактов
    
    // счётчики
    logic [CNT_WIDTH-1:0] bit_timer;
    logic [3:0] bit_counter;
    
    // регистры
    logic [10:0] tx_shift_reg; // сдвиговый регистр: стоп, чётность, данные, старт
    logic tx_parity_bit; // вычисленный бит чётности
	 
    // флаги управления
    logic tx_start_detected; // флаг обнаружения сигнала старта
    logic tx_start_prev; // предидущее значение tx_start
    
    // ----------------------------------------------------------
    // СИНХРОНИЗАЦИЯ ВЫХОДНОГО СИГНАЛА
    // ----------------------------------------------------------
    logic tx_out_reg; // регистр для выходного сигнала
    logic tx_out_sync; // синхронизированный выход
    
    // ----------------------------------------------------------
    // отслеживаем начала передачи
    // ----------------------------------------------------------
    always_ff @(posedge clk or negedge reset) begin
        if (!reset) begin
            tx_start_prev <= 1'b0; // сброс
        end else begin
            tx_start_prev <= tx_start; // запоминаем текущее значение
        end
    end
    
	 // постоянно вычисляем, чтобы начало было 1 а до 0, тогда начало
    assign tx_start_detected = tx_start && !tx_start_prev; 
    
    // ----------------------------------------------------------
    // основная логика передачи
    // ----------------------------------------------------------
    always_ff @(posedge clk or negedge reset) begin
        if (!reset) begin // сброс
            tx_busy <= 1'b0;
            tx_done <= 1'b0;
            bit_timer <= '0;
            bit_counter <= '0;
            tx_out_reg <= 1'b1; // линия в состоянии покоя = 1 (регистр)
            tx_shift_reg <= 11'b11111111111;  // все стоп биты
        end else begin
            // сброс сигнала завершения
            tx_done <= 1'b0;
            
            // если обнаружен старт передачи и передатчик свободен
            if (tx_start_detected && !tx_busy) begin
                tx_busy <= 1'b1; // занят
                bit_timer <= '0; // обнуляем счётчик тактов
                bit_counter <= 4'd0; // начинаем отпраку с 0 бита
                
                // выччисление бита чётности
                tx_parity_bit = ^tx_data;  // XOR всех битов данных
                
                //формирование пакета [1 (стоп), чётность, данные[7:0], 0 (старт)]
                // передаём с младшео бита, то есть с конца
                tx_shift_reg <= {1'b1, tx_parity_bit, tx_data, 1'b0};
                tx_out_reg <= 1'b0; // начинаем передачу со стартового бита (0)
            end
            
            // если идёт передача
            if (tx_busy) begin
                bit_timer <= bit_timer + 1;// увеличиваем кол-во тактов
                
                // достигли конца текущего бита
                if (bit_timer == TICKS_PER_BIT - 1) begin
                    bit_timer <= '0; // обнуляем счётчик
                    bit_counter <= bit_counter + 1; // переходим к след. биту
                    
                    // если не все биты отправленны 
                    if (bit_counter < 4'd10) begin
                        // сдвигаем регистр и отправляем следующий бит
                        tx_shift_reg <= {1'b1, tx_shift_reg[10:1]}; // сдвиг от младщего к старшему было 1 0 0 1 0 0 0 0 0 0 (1 этот отправляем) 0
								                                                                          // стало 1 1 0 0 1 0 0 0 0 0 (0 этот отправляем) 1
                        tx_out_reg <= tx_shift_reg[1];  // отправка бита из регистра
                    end else begin
                        // если передали все биты
                        tx_busy <= 1'b0; // свободен
                        tx_done <= 1'b1; // завершение отправки
                        tx_out_reg <= 1'b1; // возвращение линии в состояние покоя = 1
                    end
                end
            end
        end
    end
    
    // ----------------------------------------------------------
    // СИНХРОНИЗАЦИЯ ВЫХОДНОГО СИГНАЛА
    // ----------------------------------------------------------
    // Дополнительный регистр для синхронизации выхода
    // Это гарантирует чистый сигнал на выходе без глитчей
    always_ff @(posedge clk or negedge reset) begin
        if (!reset) begin
            tx_out_sync <= 1'b1; // при сбросе линия в покое
        end else begin
            tx_out_sync <= tx_out_reg; // синхронизация выхода
        end
    end
    
    // Назначение синхронизированного выхода
    assign tx_out = tx_out_sync;

endmodule