library verilog;
use verilog.vl_types.all;
entity uart_echo_top is
    port(
        system_clock    : in     vl_logic;
        reset_button    : in     vl_logic;
        uart_receive_in : in     vl_logic;
        uart_transmit_out: out    vl_logic
    );
end uart_echo_top;
