library verilog;
use verilog.vl_types.all;
entity uart_vlg_check_tst is
    port(
        uart_tx         : in     vl_logic;
        sampler_rx      : in     vl_logic
    );
end uart_vlg_check_tst;
