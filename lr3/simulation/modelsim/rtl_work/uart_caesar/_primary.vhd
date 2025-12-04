library verilog;
use verilog.vl_types.all;
entity uart_caesar is
    generic(
        CLK_FREQ        : integer := 50000000;
        BAUD            : integer := 6250000;
        SHIFT           : integer := 3
    );
    port(
        clk50           : in     vl_logic;
        reset_n         : in     vl_logic;
        uart_rx         : in     vl_logic;
        uart_tx         : out    vl_logic
    );
    attribute mti_svvh_generic_type : integer;
    attribute mti_svvh_generic_type of CLK_FREQ : constant is 1;
    attribute mti_svvh_generic_type of BAUD : constant is 1;
    attribute mti_svvh_generic_type of SHIFT : constant is 1;
end uart_caesar;
