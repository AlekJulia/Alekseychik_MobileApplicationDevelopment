// приёмник: 0 [8 бит данных] бит чётности 1 
module uart_receiver #(
    parameter CLK_FREQ = 50_000_000, // 50 Мгц
    parameter BAUD_RATE = 115200 // бод рейт (115200 бит в сек)
)(
    input  clk,
    input  reset,
    input  rx_in, // вход данных
    output reg [7:0] rx_data, // данные
    output reg rx_valid, //сигнал, что данные готовы
    output reg rx_error // сигнал об ошибке
);

    localparam integer TICKS_PER_BIT = CLK_FREQ / BAUD_RATE; // кол-во тактов на 1 бит (434)
    localparam integer HALF_BIT_TICKS = TICKS_PER_BIT / 2; // тактов до середины бита (чтобы считывать бит в середине)
    localparam integer CNT_WIDTH = $clog2(TICKS_PER_BIT); // кол-во битов для счётчика (9 битный счётчик (2^9), чтобы уместить 434 такта)
    
    // ----------------------------------------------------------
    // СИНХРОНИЗАЦИЯ ВХОДНОГО СИГНАЛА (устранение метастабильности)
    // ----------------------------------------------------------
    reg rx_sync_0, rx_sync_1; 
    always_ff @(posedge clk) begin
        if (!reset) begin
            rx_sync_0 <= 1'b1; // при сбросе в 1 (линия в покое)
            rx_sync_1 <= 1'b1;
        end else begin
            rx_sync_0 <= rx_in; // новое значение
            rx_sync_1 <= rx_sync_0; // СТАРОЕ значение rx_sync_0 (задержка на один такт!)
        end
    end
    wire rx_synced = rx_sync_1; // синхронизированный сигнал rx
    
    // счётчики
    reg [CNT_WIDTH-1:0] bit_timer; // 9 битный счётчик для тактов
    reg [3:0] bit_counter; // счётчик для подсчёта битов
    
    // регистры данных
    reg [9:0] shift_reg; // сдвиговй регистр, для сбора всех битов: старт(0), данные[7:0], чётность
    reg rx_parity_calc; // вычисленная четность
    reg rx_parity_received; // принятая чётность
    reg rx_stop_bit; // принятый стоп бит (всегда = 1)
    
    // флаги состояний
    reg receiving; // 1 - принятие данных, 0 - ожидание
    reg do_sample; // 1 - середина бита, считывание


    // ----------------------------------------------------------
    // обнаружение стартового бита
    // ----------------------------------------------------------
    reg rx_prev; // регистр для хранения предыдущего значения
    always @(posedge clk or negedge reset) begin 
        if (!reset) begin
            rx_prev <= 1'b1; // при сбросе ставим 1, uart в покое
        end else begin
            rx_prev <= rx_synced; // запоминаем текущее значение СИНХРОНИЗИРОВАННОГО сигнала
        end
    end
    
    wire start_detected; // следим когда появится старт бит
	 // постоянно вычисляем условие, если до была 1 а потом 0, то пришёл стартовый бит
    assign start_detected = (rx_prev == 1'b1) && (rx_synced == 1'b0);
    
    // ----------------------------------------------------------
    // основной бок получения данных
    // ----------------------------------------------------------
    always @(posedge clk or negedge reset) begin
        if (!reset) begin
            // сброс всех регистров
            bit_timer <= 0;
            bit_counter <= 0;
            receiving <= 0;
            do_sample <= 0;
            shift_reg <= 0;
            rx_data <= 0;
            rx_valid <= 0;
            rx_error <= 0;
            rx_parity_calc <= 0;
            rx_parity_received <= 0;
            rx_stop_bit <= 1; // линия в покое
				
        end else begin
            // сброс выходных флагов
            rx_valid <= 0; // данные ещё не готовы
            rx_error <= 0; // ошибки ещё нет
            
            do_sample <= 0; // считывания ещё нет
            
            // обнаружения стартового бита (стартовый бит обнаружен, принятие ещё не началось)
            if (start_detected && !receiving) begin 
                receiving <= 1; // начало принятия
                bit_timer <= 0; // обнуление счётчика
                bit_counter <= 0; // обнуление счётчика битов
                shift_reg <= 0; // отчищение регистра 
                rx_parity_calc <= 0; // обнуление расчёта точности
            end
            
            // если начали принимать
            if (receiving) begin
                bit_timer <= bit_timer + 1; // увеличение счётчика
                
                // если дошли до середины бита
                if (bit_timer == HALF_BIT_TICKS - 1) begin
                    do_sample <= 1; // устанавливаем флаг, чтобы считать значение
                end
                
                // считываение данных в середине каждого бита
                if (do_sample) begin
                    case (bit_counter)
                        4'd0: begin  // проверка, что стартовый бит = 0
                            if (rx_synced != 1'b0) begin // используем СИНХРОНИЗИРОВАННЫЙ сигнал
                                // если ошибка, то отмена приёма
                                receiving <= 0;
                            end
                        end
                        
                        4'd1: begin  // бит данных 0
                            shift_reg[0] <= rx_synced; // считываине бита в ячейку
                            rx_parity_calc <= rx_synced; // для подсчёта чётности бита
                        end
                        
                        4'd2: begin  // бит данных 1
                            shift_reg[1] <= rx_synced;
                            rx_parity_calc <= rx_parity_calc ^ rx_synced; //(xor 0 ^ 0 = 1, 1 ^ 1 = 0, 0 ^ 1 = 1, подсчёт чётности)
                        end
                        
                        4'd3: begin  // бит данных 2
                            shift_reg[2] <= rx_synced;
                            rx_parity_calc <= rx_parity_calc ^ rx_synced;
                        end
                        
                        4'd4: begin  // бит данных 3
                            shift_reg[3] <= rx_synced;
                            rx_parity_calc <= rx_parity_calc ^ rx_synced;
                        end
                        
                        4'd5: begin  // бит данных 4
                            shift_reg[4] <= rx_synced;
                            rx_parity_calc <= rx_parity_calc ^ rx_synced;
                        end
                        
                        4'd6: begin  // бит данных 5
                            shift_reg[5] <= rx_synced;
                            rx_parity_calc <= rx_parity_calc ^ rx_synced;
                        end
                        
                        4'd7: begin  // бит данных 6
                            shift_reg[6] <= rx_synced;
                            rx_parity_calc <= rx_parity_calc ^ rx_synced;
                        end
                        
                        4'd8: begin  // бит данных 7 (последний)
                            shift_reg[7] <= rx_synced;
                            rx_parity_calc <= rx_parity_calc ^ rx_synced;
                        end
                        
                        4'd9: begin  // бит чётности
                            shift_reg[8] <= rx_synced;
                            rx_parity_received <= rx_synced; // сохраняем для проверки
                        end
                        
                        4'd10: begin // стоп бит (= 1)
                            rx_stop_bit <= rx_synced;
                        end
                        
                        default: begin
                        end
                    endcase
                end
                
                // конец бита и переход к следующему 
                if (bit_timer == TICKS_PER_BIT - 1) begin // если дошли до конца бита
                    bit_timer <= 0; // обнуление таймера
                    bit_counter <= bit_counter + 1; // переход к следующему биту
                    
                    // проверка 10 бита - конец пакета = 1
                    if (bit_counter == 4'd10) begin
                        // проверка стоп бита
                        if (rx_stop_bit == 1'b1) begin
                            // проверка чётности
                            if (rx_parity_calc == rx_parity_received) begin
                                rx_data <= shift_reg[7:0];  // извлечение 8 бит данных
                                rx_valid <= 1; // данные без ошибок
                            end else begin
                                rx_error <= 1; // иначе ошибка чётности
                            end
                        end else begin
                            rx_error <= 1; // если ошибка в стоп бите
                        end
                        receiving <= 0; // завершение приёма
                    end
                end
            end
        end
    end

endmodule