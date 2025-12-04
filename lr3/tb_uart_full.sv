`timescale 1ns/1ps // устанавливаем временной масштаб в нс (точность пикосекунды)

module tb_uart_full; // объявление тестового модуля

    // ----------------------------
    // Временные параметры симуляции
    // ----------------------------
    parameter CLK_PERIOD  = 20;   // период clk 20нс -> freq = 50 MHz
    parameter BAUD_PERIOD = 160;  // 6.25 Mbaud, 1 bit = 160 ns

    // ----------------------------
    // Основные сигналы
    // ----------------------------
    reg clk;
    reg reset;
    reg uart_rx; // вход 
    wire uart_tx; // выход

    // Внутренние сигналы
    wire rx_ready; // сигнал готовности приемника
    wire tx_busy; // сигнал занятости передатчика
    wire [7:0] rx_byte; // принятые данные 8 бит

    integer i; // переменная для обозначения индекса в циклах
    integer error_count; // переменная для накопления обнаруженных ошибок

    // ----------------------------
    // Создание экземпляра устройства для тестирования
    // ----------------------------
    uart uart1 (
        .clk50   (clk), // .port_name_in_module (signal_name_in_testbench)
        .reset_n (~reset),
        .uart_rx (uart_rx),
        .uart_tx (uart_tx)
    );

    // Внутренние сигналы для отладки
    assign rx_ready = uart1.rx_ready;
    assign tx_busy = uart1.tx_busy;
    assign rx_byte = uart1.rx_byte;

    // ----------------------------
    // Генерация тактового сигнала
    // ----------------------------
    initial clk = 0;
    always #(CLK_PERIOD/2) clk = ~clk;

    // -----------------------------
    // Начальный сброс 
    // -----------------------------
    initial begin // initial блок выполняется 1 раз при старте симуляции (не имеет параметров + не возвращает значение)
        reset   = 1;
        uart_rx = 1;     // idle
        error_count = 0;
        #200;
        reset = 0;
    end

    // ------------------------------
    // TASK: Отправка UART байта
    // ----------------------------
    task send_uart_byte; // таск - функция
        input [7:0] b; // входной параметр - 8 бит данных
        integer j; 
        reg parity;
        begin
            parity = ^b;   // even parity

            // START bit
            uart_rx = 0;
            #(BAUD_PERIOD);

            // DATA bits
            for (j=0; j<8; j=j+1) begin
                uart_rx = b[j];
                #(BAUD_PERIOD);
            end

            // PARITY bit
            uart_rx = parity;
            #(BAUD_PERIOD);

            // STOP bit
            uart_rx = 1;
            #(BAUD_PERIOD);
        end
    endtask

    // ----------------------------
    // TASK: Побитовая проверка полученных данных
    // ----------------------------
    task check_uart_byte;
        input [7:0] expected_byte; // входной параметр: ожидаемый байт для сравнения
        integer k;
        reg expected_parity; // вычисленный ожидаемый бит чётности
        reg received_bit; // текущий принятый бит с линии TX
        begin
            expected_parity = ^expected_byte; // высчитваем бит четности even

            $display("=== Starting bit-by-bit verification of byte 0x%h ===", expected_byte);

            // Ожидание START bit = 0
            wait(uart_tx === 0);
            $display("Time %0t ns: START bit received", $time);
            #(BAUD_PERIOD/2); // Ждем центра бита (для исключения метастабильности)

            // Проверка битов данных (LSB first)
            for (k=0; k<8; k=k+1) begin
                #(BAUD_PERIOD);
                received_bit = uart_tx; 
                if (received_bit !== expected_byte[k]) begin // если биты не совпали
                    $error("Error in bit %0d: expected %b, received %b", 
                           k, expected_byte[k], received_bit);
                    error_count = error_count + 1;
                end else begin
                    $display("Time %0t ns: Bit %0d = %b", $time, k, received_bit);
                end
            end

            // Проверка бита четности PARITY bit
            #(BAUD_PERIOD);
            received_bit = uart_tx;
            if (received_bit !== expected_parity) begin
                $error("Error in parity bit: expected %b, received %b", 
                       expected_parity, received_bit);
                error_count = error_count + 1;
            end else begin
                $display("Time %0t ns: Parity bit = %b", $time, received_bit);
            end

            // Проверка стопового бита STOP bit = 1
            #(BAUD_PERIOD);
            received_bit = uart_tx;
            if (received_bit !== 1'b1) begin
                $error("Error in STOP bit: expected 1, received %b", received_bit);
                error_count = error_count + 1;
            end else begin
                $display("Time %0t ns: STOP bit = %b", $time, received_bit);
            end

            $display("=== Verification of byte 0x%h completed ===", expected_byte);
        end
    endtask

    // --------------------------------------------------------------------------------------
    // Отследивание состояний uart для вывода текущего состояния завершения rx или начала tx
    // --------------------------------------------------------------------------------------
    initial begin
        forever begin // бесконечный цикл 
            @(posedge rx_ready); // блокировка в виде ожидания события
            $display("Time %0t ns: RX received byte 0x%h", $time, rx_byte);
        end
    end

    initial begin
        forever begin
            @(posedge tx_busy);
            $display("Time %0t ns: TX started transmission", $time);
        end
    end

    // ----------------------------
    // Основная симуляция
    // ----------------------------
    initial begin
        #500;

        $display("=== Starting UART loopback test ===");

        // Test 1: Отправка rx 0x55 и проверка на tx 
        fork // процессы запускаются параллельно
            send_uart_byte(8'h55);
            check_uart_byte(8'h55);
        join // блокирует выполнение до тех пор, пока ОБА процесса не завершатся

        #400;

        // Test 2: Отправка rx 0xAA и проверка на tx 
        fork
            send_uart_byte(8'hAA);
            #600 check_uart_byte(8'hAA);
        join

        #400;

        // Test 3: Отправка rx 0x00 и проверка на tx 
        //fork
            //send_uart_byte(8'h00);
            //#600 check_uart_byte(8'h00);
        //join

        //#400;

        // Test 4: Отправка rx 0xFF и проверка на tx 
        //fork
            //send_uart_byte(8'hFF);
            //#600 check_uart_byte(8'hFF);
        //join

        //#400;

        // Вывод после выполнения тестов
        if (error_count == 0) begin
            $display("=== TEST PASSED SUCCESSFULLY! Errors: %0d ===", error_count);
        end else begin
            $display("=== TEST FAILED! Errors: %0d ===", error_count);
        end

        $stop;
    end

endmodule