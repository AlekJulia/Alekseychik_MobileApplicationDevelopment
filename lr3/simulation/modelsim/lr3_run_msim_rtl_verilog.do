transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -sv -work work +incdir+C:/Users/Ula/Documents/GitHub/Alekseychik_MobileApplicationDevelopment/lr3 {C:/Users/Ula/Documents/GitHub/Alekseychik_MobileApplicationDevelopment/lr3/uart_caesar_cipher.sv}
vlog -sv -work work +incdir+C:/Users/Ula/Documents/GitHub/Alekseychik_MobileApplicationDevelopment/lr3 {C:/Users/Ula/Documents/GitHub/Alekseychik_MobileApplicationDevelopment/lr3/flow_control.sv}
vlog -sv -work work +incdir+C:/Users/Ula/Documents/GitHub/Alekseychik_MobileApplicationDevelopment/lr3 {C:/Users/Ula/Documents/GitHub/Alekseychik_MobileApplicationDevelopment/lr3/uart_echo.sv}
vlog -sv -work work +incdir+C:/Users/Ula/Documents/GitHub/Alekseychik_MobileApplicationDevelopment/lr3 {C:/Users/Ula/Documents/GitHub/Alekseychik_MobileApplicationDevelopment/lr3/uart_receiver.sv}
vlog -sv -work work +incdir+C:/Users/Ula/Documents/GitHub/Alekseychik_MobileApplicationDevelopment/lr3 {C:/Users/Ula/Documents/GitHub/Alekseychik_MobileApplicationDevelopment/lr3/uart_transmitter.sv}
vlog -sv -work work +incdir+C:/Users/Ula/Documents/GitHub/Alekseychik_MobileApplicationDevelopment/lr3 {C:/Users/Ula/Documents/GitHub/Alekseychik_MobileApplicationDevelopment/lr3/uart_echo_top.sv}

