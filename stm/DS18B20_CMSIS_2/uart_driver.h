#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "stm32f10x.h"

// Конфигурация UART
#define UART_BUFFER_SIZE 100
#define UART_BAUDRATE 9600

// Буферы и флаги
extern unsigned char uart_rx_buffer[UART_BUFFER_SIZE];
extern volatile uint8_t uart_string_received;
extern int uart_rx_index;

// Базовые функции UART
void UART_Init(void);
void UART_SendChar(unsigned char symbol);
void UART_SendString(const char *str);
void UART_SendNumber(int number);
int UART_ReceiveChar(unsigned char *data);
void UART_ProcessReception(void);

// Вспомогательные функции
void UART_SendPrompt(void);

#endif