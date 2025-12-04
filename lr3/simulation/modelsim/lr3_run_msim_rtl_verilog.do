transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -sv -work work +incdir+D:/study/vhdl/lr3 {D:/study/vhdl/lr3/uartrx.sv}
vlog -sv -work work +incdir+D:/study/vhdl/lr3 {D:/study/vhdl/lr3/uarttx.sv}
vlog -sv -work work +incdir+D:/study/vhdl/lr3 {D:/study/vhdl/lr3/caesar_cipher.sv}
vlog -sv -work work +incdir+D:/study/vhdl/lr3 {D:/study/vhdl/lr3/uart_caesar.sv}

