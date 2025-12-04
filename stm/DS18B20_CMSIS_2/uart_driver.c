#include "uart_driver.h"
#include <string.h>

// буфер для приёма строки
unsigned char uart_rx_buffer[UART_BUFFER_SIZE];
// флаг завершения приёма строки
volatile uint8_t uart_string_received = 0;
// индекс позиции в буфере
int uart_rx_index = 0;

// инициализация UART
void UART_Init(void) {
    // включение тактирования
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;  // USART2
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;    // GPIOA
    
    // настройка PA2 (TX) - альтернативная функция, push-pull (может переключать и 1 и 0)
    GPIOA->CRL &= ~GPIO_CRL_CNF2_0;
    GPIOA->CRL |= (GPIO_CRL_CNF2_1 | GPIO_CRL_MODE2);
    
    // настройка PA3 (RX) - вход с pull-up (поддтягивает к 1, когда ничего не происходит)
    GPIOA->CRL &= ~GPIO_CRL_CNF3_0;
    GPIOA->CRL |= GPIO_CRL_CNF3_1;
    GPIOA->CRL &= ~GPIO_CRL_MODE3;
    GPIOA->BSRR |= GPIO_ODR_ODR3;
    
		// внутри uart считает в 16 раз быстрее чтобы точнее ловить биты?
    // настройка скорости (72 МГц / (16 * 9600) = 468.75)
	  // 7500 это 468.75 в особом формате (чё за формат?)
    USART2->BRR = 7500;
	
    //                      TX         RX   весь модуль
    // включение USART: передатчик, приёмник, USART
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
		// остальные настройки оставляем по умолчанию
    USART2->CR2 = 0;
    USART2->CR3 = 0;
}

// отправка одного символа
void UART_SendChar(unsigned char symbol) {
		// SR - статус регистр
    while ((USART2->SR & USART_SR_TXE) == 0) {}  // ждём готовности
		// если добавить символ в регистр DR то uart его отправит
    USART2->DR = symbol;
}

// Отправка строки
void UART_SendString(const char *str) {
    while (*str) {
        UART_SendChar(*str);
        str++;
    }
}

// отправка числа
void UART_SendNumber(int number) {
    char buffer[20]; // буфер для цифр
    int i = 0; // счётчик цифр
    int is_negative = 0; // для проверки негативное ли число
    
    if (number < 0) {
        is_negative = 1; // сохраняем минус если негативное
        number = -number; // но для передачи делаем его положительным
    }
    
    // преобразуем число в строку (в обратном порядке, что бы было удобнее, так как мы берём остаток от деления)
    do {
        buffer[i++] = (number % 10) + '0'; // прибавляем символ '0' в ASCII 51, получаем код ASCII нужной цифры
        number /= 10;
    } while (number > 0);
    
		// если число было негативным, то добавляем 0 в конец
    if (is_negative) {
        buffer[i++] = '-';
    }
    
    // отправляем в правильном порядке с конца
    while (i > 0) {
        UART_SendChar(buffer[--i]);
    }
}

// приём одного символа (неблокирующий так как проверяем регистры, а не ждём когда они станут != 0)
int UART_ReceiveChar(unsigned char *data) {
	  // RXNE - receive not empty
    if ((USART2->SR & USART_SR_RXNE) != 0) {
        *data = (unsigned char)USART2->DR; // достаём из регистра DR символ, который пришёл 
        return 1; // cимвол принят
    }
    return 0; // cимвола нет
}

// обработка приёма строки (вызывается в главном цикле)
void UART_ProcessReception(void) {
    unsigned char received_char;
    
	  // если получен символ
    if (UART_ReceiveChar(&received_char)) {
        // если пришёл Enter - завершение строки 
			  if (received_char == '\r') {
            if (uart_rx_index > 0) { // если что-то набрано
                uart_rx_buffer[uart_rx_index] = '\0'; // ставится метка конца строки в конец
                uart_string_received = 1; // флаг строка готова
            }
        }
				// если хотим удалить символ
				else if (received_char == '\b') {
            //'\b' = Backspace escape
            if (uart_rx_index > 0) {
                uart_rx_index--;  // возвращаемся на один символ назад
								// стираем ненужный символ (назад, пробел, назад)
                UART_SendString("\b \b"); 
            }
        }
        // получение обычного символа
        else if (uart_rx_index < UART_BUFFER_SIZE - 1) { // пока не заполнился буфер
            uart_rx_buffer[uart_rx_index++] = received_char; // кладём в буфер символ
            UART_SendChar(received_char); // отправляем по UART символ обратно (эхо), чтобы пользователь видел, что вводил
        }
        // переполнение буфера
        else {
            UART_SendString("\r\nBuffer overflow!\r\n");
            uart_rx_index = 0; // ставим индекс отправки в начало 
						UART_SendString("\r\nPlease repeat the last command.\r\n");
        }
    }
}

// отправка приглашения для ввода
void UART_SendPrompt(void) {
    UART_SendString("\r\nYOU >> ");
}