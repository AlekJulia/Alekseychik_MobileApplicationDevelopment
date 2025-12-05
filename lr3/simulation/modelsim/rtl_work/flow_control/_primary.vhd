library verilog;
use verilog.vl_types.all;
entity flow_control is
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        rx_data         : in     vl_logic_vector(7 downto 0);
        rx_valid        : in     vl_logic;
        allow_transmit  : out    vl_logic;
        is_control_char : out    vl_logic
    );
end flow_control;
