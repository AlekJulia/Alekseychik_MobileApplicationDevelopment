// uartrx.sv — устойчивый приёмник, 1 start, 8 data, even parity, 1 stop
module uartrx #(
	 parameter CLK_FREQ = 50_000_000, 
    //parameter BAUD     = 6_250_000 // символьная скорость 1б/с
    //parameter CLK_FREQ = 50_000_000,
    parameter BAUD     = 115200
)(
    input  logic clk,
    input  logic reset,    // должен быть активным при 0
    input  logic rx,       // асинхронный внешний сигнал последовательные данные, которые приемник получает 
    output logic data_valid, // флаг валидности принятых данныз
    output logic [7:0] data // 8 бит принятых данных
);

    localparam integer DIVIDER = CLK_FREQ / BAUD; // делитель частоты
	 localparam integer CNT_WIDTH = $clog2(DIVIDER); 

    // ----------------------------------------------------------
    // 2-stage synchronizer для входа RX (устраняет метастабильность)
    // ----------------------------------------------------------
    logic rx_sync_0, rx_sync_1; 
    always_ff @(posedge clk) begin
        rx_sync_0 <= rx; // новое значение
        rx_sync_1 <= rx_sync_0; // СТАРОЕ значение rx_sync_0 (задержка на один такт!)
    end
    wire rxs = rx_sync_1;

    // ----------------------------------------------------------
    // Baudrate generator
    // ----------------------------------------------------------
    logic [CNT_WIDTH-1:0] baud_cnt; //счетчик делителя частоты 
    logic  baud_tick; // импульс длительностью 1 такт для считывания 1 бита данных
    logic  half_start; // импульс от мс: начать счёт с середины бита для устранения метастабильности

    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            baud_cnt  <= '0;
            baud_tick <= 1'b0;
        end else begin
            if (half_start) begin // обнаружение старт бита на фронте (в самом начале в мс -> синхронизация таймера защита от метастабильности
                baud_cnt  <= DIVIDER/2; // Полный период бита UART = DIVIDER тактов -> центр бита = DIVIDER/2 тактов от начала
                baud_tick <= 1'b0; // защита от случайного срабатывания, тк тик должен срабатывать только при полном завершении таймера
            end
            else if (baud_cnt == DIVIDER-1) begin
                baud_cnt  <= '0;
                baud_tick <= 1'b1;
            end
            else begin
                baud_cnt  <= baud_cnt + 1;
                baud_tick <= 1'b0;
            end
        end
    end

    // ----------------------------------------------------------
    // FSM
    // ----------------------------------------------------------
    typedef enum logic [2:0] { IDLE, START, BITS, PAR, STOP } state_t;
    state_t state; // объявление переменной текущего состояния

    logic [7:0] rxbuf; // буфер для накопления принятых битов
    logic [3:0] bitpos; // счетчик принятых битов
    logic parity_calc; // расчет бита четности

    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            state       <= IDLE;
            data_valid  <= 1'b0; // снятие флага валидности данных
            data        <= 8'd0; // сбрасывание выходного регистра с битами данных, чтобы он не сохранил старые значения
            rxbuf       <= 8'd0; // очищение буфера приема
            bitpos      <= 4'd0; // сбрасываем счетчик битов данных 8бит 
            parity_calc <= 1'b0; 
            half_start  <= 1'b0; // запрет коррекции фазы таймера
        end else begin
            data_valid <= 1'b0;
            half_start <= 1'b0;

            case (state) // выбор текущего состояния

                IDLE: begin
                    if (rxs == 1'b0) begin // если получили стартовый бит 
                        half_start <= 1'b1;
                        state <= START;
                    end
                end

                START: begin
                    if (baud_tick) begin // ожидание тактового импульса от генератора бодрейта
                        if (rxs == 1'b0) begin 
                            bitpos      <= 4'd0;
                            parity_calc <= 1'b0;
                            state <= BITS;
                        end else begin
                            state <= IDLE;
                        end
                    end
                end

                BITS: begin
                    if (baud_tick) begin
                        rxbuf[bitpos] <= rxs; // сохранение текущего бита в буфер приёма
                        parity_calc   <= parity_calc ^ rxs; // обновление бита четности even XOR

                        if (bitpos == 4'd7) // проверка все ли биты приняты
                            state <= PAR; // аереходим в состояние проверки бита четности

                        bitpos <= bitpos + 1;
                    end
                end

                PAR: begin
                    if (baud_tick) begin
                        if (rxs == parity_calc)
                            state <= STOP;
                        else
                            state <= IDLE;
                    end
                end

                STOP: begin
                    if (baud_tick) begin
                        if (rxs == 1'b1) begin
                            data <= rxbuf; // копируем данные из буфера в выходной регистр
                            data_valid <= 1'b1; // данные готовы для передачи
                        end
                        state <= IDLE;
                    end
                end
					 
					 default: state <= IDLE; // состояние по умолчанию

            endcase
        end
    end

endmodule
