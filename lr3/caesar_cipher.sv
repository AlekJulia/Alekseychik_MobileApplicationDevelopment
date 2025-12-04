// ------------------------------------------------------------
// Caesar cipher encoder
// Только буквы A–Z и a–z, регистр сохраняется
// SHIFT — величина сдвига (по умолчанию 3)
// ------------------------------------------------------------
module caesar_cipher #(
    parameter integer SHIFT = 3
)(
    input  logic [7:0] in_char,   // входящий байт ASCII
    output logic [7:0] out_char   // зашифрованный байт ASCII
);

    always_comb begin
        // по умолчанию — символ без изменений
        out_char = in_char;

        // ---- Uppercase A..Z ----
        if (in_char >= "A" && in_char <= "Z") begin
            out_char = "A" + ((in_char - "A" + SHIFT) % 26);
        end

        // ---- Lowercase a..z ----
        else if (in_char >= "a" && in_char <= "z") begin
            out_char = "a" + ((in_char - "a" + SHIFT) % 26);
        end
    end

endmodule
