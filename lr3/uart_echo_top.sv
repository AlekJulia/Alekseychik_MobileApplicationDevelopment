// константы для напряжения на выводах плис
`define VOLTAGE_1_2 "-name IO_STANDARD \"1.2-V\""
`define VOLTAGE_2_5 "-name IO_STANDARD \"2.5-V\""

// пивязка сигналов к физическим пинам
`define PIN_CLOCK     "H12"   // выывод тактового сигнала (50 МГц)
`define PIN_RESET_BTN "P11"   // вывод кнопки сброса
`define PIN_UART_RX   "M9"    // вывод приёма данных UART
`define PIN_UART_TX   "L9"    // вывод передачи данных UART

// ============================================================================
// верхний модуль
// ============================================================================
module uart_echo_top(
    // тактовый сигнал 50 МГц 
    (* chip_pin = `PIN_CLOCK *) 
    input wire system_clock,
    
    (* altera_attribute = `VOLTAGE_1_2, chip_pin = `PIN_RESET_BTN *) 
    input wire reset_button,
    
    (* altera_attribute = `VOLTAGE_2_5, chip_pin = `PIN_UART_RX *) 
    input wire uart_receive_in,
    
    (* altera_attribute = `VOLTAGE_2_5, chip_pin = `PIN_UART_TX *) 
    output wire uart_transmit_out
);

    // ========================================================================
    // подключение основного модуля uart
    // ========================================================================
    
    uart_echo #(
        .CLK_FREQ(50_000_000),
        .BAUD_RATE(115200)     
    ) uart_echo_controller (
        .clk(system_clock),       
        .reset(reset_button), 
        .uart_rx(uart_receive_in),    
        .uart_tx(uart_transmit_out)  
    );

endmodule