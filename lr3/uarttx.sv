// uarttx.sv — устойчивый передатчик, 1 start, 8 data, even parity, 1 stop
module uarttx #(
	 parameter CLK_FREQ = 50_000_000,
    //parameter BAUD     = 6_250_000
    //parameter CLK_FREQ = 50_000_000,
    parameter BAUD     = 115200
)(
    input  logic clk,
    input  logic reset,    // active-high
    input  logic start,    // 1 импульс запуска передачи
    input  logic [7:0] data, // 8 бит данных для передачи
    output logic tx, // выходная линия uart
    output logic busy // флаг что передатчик занят
);

    localparam integer DIVIDER = CLK_FREQ / BAUD;
	 localparam integer CNT_WIDTH = $clog2(DIVIDER); 


    // -----------------------------
    // Baud generator (единственный драйвер cnt)
    // -----------------------------
    logic [CNT_WIDTH-1:0] cnt;
    logic baud_tick;

    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            cnt <= '0;
            baud_tick <= 1'b0;
        end else begin
            if (cnt == DIVIDER - 1) begin
                cnt <= '0;
                baud_tick <= 1'b1;
            end else begin
                cnt <= cnt + 1;
                baud_tick <= 1'b0;
            end
        end
    end

    // -----------------------------
    // start edge detector (встроен, но лучше давать single-cycle start) // если start активен больше 1 такта, передатчик запустится
    // -----------------------------
    logic start_d;
    always_ff @(posedge clk or posedge reset) begin
        if (reset) start_d <= 1'b0;
        else start_d <= start; // 0
    end
    wire start_pulse = start & ~start_d;  // 1 & !0

    // -----------------------------
    // Shifter: 1 start + 8 data + 1 parity + 1 stop = 11 bits
    // -----------------------------
    logic [10:0] shifter;
    logic [3:0]  bits_left; // счетчик оставшихся битов для передачи

    assign busy = (bits_left != 0);

    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            tx <= 1'b1; // idle = 1 линия uart в состоянии покоя
            shifter <= 11'h7FF; // 
            bits_left <= 4'd0;
        end else begin
            // запуск передачи по фронту start_pulse
            if (start_pulse && !busy) begin
                logic parity_bit;
                parity_bit = ^data; // even parity
                // формируем фрейм: stop | parity | data[7:0] | start
                shifter <= {1'b1, parity_bit, data, 1'b0};
                bits_left <= 4'd11; // устанавливаем счетчик 11 бит для передачи
            end

            // при тике baud — сдвиг
            else if (baud_tick && busy) begin
                tx <= shifter[0];
                shifter <= {1'b1, shifter[10:1]}; // старший бит становится 1 (слева), самый младший бит убираем
                bits_left <= bits_left - 1;
            end
        end
    end

endmodule
