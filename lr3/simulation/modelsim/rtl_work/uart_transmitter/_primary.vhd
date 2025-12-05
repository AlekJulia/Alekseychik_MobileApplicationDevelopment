library verilog;
use verilog.vl_types.all;
entity uart_transmitter is
    generic(
        CLK_FREQ        : integer := 50000000;
        BAUD_RATE       : integer := 115200
    );
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        tx_start        : in     vl_logic;
        tx_data         : in     vl_logic_vector(7 downto 0);
        tx_out          : out    vl_logic;
        tx_busy         : out    vl_logic;
        tx_done         : out    vl_logic
    );
    attribute mti_svvh_generic_type : integer;
    attribute mti_svvh_generic_type of CLK_FREQ : constant is 1;
    attribute mti_svvh_generic_type of BAUD_RATE : constant is 1;
end uart_transmitter;
