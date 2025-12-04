library verilog;
use verilog.vl_types.all;
entity uarttx is
    generic(
        CLK_FREQ        : integer := 50000000;
        BAUD            : integer := 6250000
    );
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        start           : in     vl_logic;
        data            : in     vl_logic_vector(7 downto 0);
        tx              : out    vl_logic;
        busy            : out    vl_logic
    );
    attribute mti_svvh_generic_type : integer;
    attribute mti_svvh_generic_type of CLK_FREQ : constant is 1;
    attribute mti_svvh_generic_type of BAUD : constant is 1;
end uarttx;
