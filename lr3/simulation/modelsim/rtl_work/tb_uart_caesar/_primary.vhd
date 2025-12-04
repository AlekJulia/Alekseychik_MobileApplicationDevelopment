library verilog;
use verilog.vl_types.all;
entity tb_uart_caesar is
    generic(
        CLK_PERIOD      : integer := 20;
        BAUD_PERIOD     : integer := 160;
        SHIFT           : integer := 3
    );
    attribute mti_svvh_generic_type : integer;
    attribute mti_svvh_generic_type of CLK_PERIOD : constant is 1;
    attribute mti_svvh_generic_type of BAUD_PERIOD : constant is 1;
    attribute mti_svvh_generic_type of SHIFT : constant is 1;
end tb_uart_caesar;
