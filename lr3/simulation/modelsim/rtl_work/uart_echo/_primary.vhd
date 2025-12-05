library verilog;
use verilog.vl_types.all;
entity uart_echo is
    generic(
        CLK_FREQ        : integer := 50000000;
        BAUD_RATE       : integer := 115200
    );
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        uart_rx         : in     vl_logic;
        uart_tx         : out    vl_logic
    );
    attribute mti_svvh_generic_type : integer;
    attribute mti_svvh_generic_type of CLK_FREQ : constant is 1;
    attribute mti_svvh_generic_type of BAUD_RATE : constant is 1;
end uart_echo;
