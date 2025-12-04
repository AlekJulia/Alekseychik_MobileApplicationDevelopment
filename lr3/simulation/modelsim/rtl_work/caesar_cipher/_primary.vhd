library verilog;
use verilog.vl_types.all;
entity caesar_cipher is
    generic(
        SHIFT           : integer := 3
    );
    port(
        in_char         : in     vl_logic_vector(7 downto 0);
        out_char        : out    vl_logic_vector(7 downto 0)
    );
    attribute mti_svvh_generic_type : integer;
    attribute mti_svvh_generic_type of SHIFT : constant is 2;
end caesar_cipher;
