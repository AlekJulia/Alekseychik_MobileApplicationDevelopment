library verilog;
use verilog.vl_types.all;
entity uart_caesar_cipher is
    port(
        plain_data      : in     vl_logic_vector(7 downto 0);
        is_control_char : in     vl_logic;
        cipher_data     : out    vl_logic_vector(7 downto 0)
    );
end uart_caesar_cipher;
