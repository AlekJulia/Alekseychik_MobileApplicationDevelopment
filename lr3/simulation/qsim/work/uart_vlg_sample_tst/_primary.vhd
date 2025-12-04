library verilog;
use verilog.vl_types.all;
entity uart_vlg_sample_tst is
    port(
        clk50           : in     vl_logic;
        reset_n         : in     vl_logic;
        uart_rx         : in     vl_logic;
        sampler_tx      : out    vl_logic
    );
end uart_vlg_sample_tst;
