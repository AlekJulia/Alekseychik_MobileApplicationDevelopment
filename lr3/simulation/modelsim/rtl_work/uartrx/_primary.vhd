library verilog;
use verilog.vl_types.all;
entity uartrx is
    generic(
        CLK_FREQ        : integer := 50000000;
        BAUD            : integer := 6250000
    );
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        rx              : in     vl_logic;
        data_valid      : out    vl_logic;
        data            : out    vl_logic_vector(7 downto 0)
    );
    attribute mti_svvh_generic_type : integer;
    attribute mti_svvh_generic_type of CLK_FREQ : constant is 1;
    attribute mti_svvh_generic_type of BAUD : constant is 1;
end uartrx;
