#include "stm32f10x.h"
#include "ClockConfigureHSE.h"
#include "ds18b20.h"
#include "sensor_monitor.h"
#include "uart_driver.h"

// для двух датчиков
#define MAX_SENSORS 10 

volatile uint8_t system_initialized = 0;  // флаг полной инициализации системы

// массив структур для хранения данных о датчиках
Sensor sensors[MAX_SENSORS]; 	
uint8_t devCount = 0;

// Переменные для измерений
uint8_t auto_measurement_enabled = 0;
uint8_t current_sensor = 0;           // Текущий измеряемый датчик
uint32_t measurement_start_time = 0;  // Время начала измерения
uint8_t measurement_in_progress = 0;  // Флаг измерения
uint8_t measurement_phase = 0;        // Фаза измерения: 0-ожидание, 1-чтение

// инициализация датчиков (устанавливаем всё в 0)
void Init_Sensors () {
    for (uint8_t i = 0; i < MAX_SENSORS; i++) {
        sensors[i].raw_temp = 0x0;
        sensors[i].temp = 0.0;
        sensors[i].crc8_rom = 0x0;
        sensors[i].crc8_data = 0x0;
        sensors[i].crc8_rom_error = 0x0;
        sensors[i].crc8_data_error = 0x0;
        
        sensors[i].is_present = 1;
        sensors[i].missing_counter = 0;
				sensors[i].alert_was_sent = 0;
        
        for (uint8_t j = 0; j < 8; j++) {
            sensors[i].ROM_code[j] = 0x00;
        }
        for (uint8_t j = 0; j < 9; j++) {
            sensors[i].scratchpad_data[j] = 0x00;
        }
    }
}

// ==================== работа с UART =========================
//              (перенести в отдельный модуль)


// вывод температуры датчика с проверкой ошибок
void PrintSensorTemperature(uint8_t sensor_index) {
	  // ошибка, если пользователь указал номер не существующего датчика
    if (sensor_index >= devCount) {
        UART_SendString("\r\nError: Sensor ");
        UART_SendNumber(sensor_index + 1);
        UART_SendString(" not found");
        return;
    }
    
		// вывод температуры
    UART_SendString("\r\nSensor ");
    UART_SendNumber(sensor_index + 1);
    UART_SendString(": ");
    
		// еcли датчика нет
    if (!sensors[sensor_index].is_present) {
        UART_SendString("MISSING");
        return;
    }
    
		// если ошибка CRC
    if (sensors[sensor_index].crc8_data_error) {
        UART_SendString("CRC ERROR");
        return;
    }
    
    // вывод температуры в формате XX.XX°C
		// смотрив температуру с созданной структуре
    int temp_int = (int)sensors[sensor_index].temp; // целая часть температуры 
    int temp_frac = (int)((sensors[sensor_index].temp - temp_int) * 100); // дробная часть температуры
    if (temp_frac < 0) temp_frac = -temp_frac; // чтобы не учитывать минус
    
		// сначала выводим целую часть
    UART_SendNumber(temp_int);
    UART_SendChar(','); 
    if (temp_frac < 10) UART_SendChar('0'); // чтобы был красивый формта 00,00
    UART_SendNumber(temp_frac);  // вывод числа
    UART_SendString("°C"); // знак градусов 
}

// измерение температуры
void MeasureTemperature(void) {
    // температуру можно мерить, только если включен start
    if (!auto_measurement_enabled) {
        return;
    }
    
    // если измерение не идет - начинаем новое
    if (!measurement_in_progress) {
        if (current_sensor < devCount) {
					  // если сенсор присутствует и нет ошибок
            if (sensors[current_sensor].is_present && !sensors[current_sensor].crc8_data_error) {
                // запускаем измерение
                ds18b20_ConvertTemp(1, sensors[current_sensor].ROM_code);
                measurement_start_time = msTicks;
                measurement_in_progress = 1;
                measurement_phase = 0; // ждем завершения измерения
            }
            current_sensor++;
        } else {
            current_sensor = 0; // Начинаем сначала
        }
    } 
    // если измерение идет - проверяем время
    else {
			//фаза 0: ждем завершения измерения (750 мс)
        if (measurement_phase == 0) {
            if ((msTicks - measurement_start_time) > 750000) {
                // измерение завершено - читаем результат из памяти
                ds18b20_ReadStratchpad(1, sensors[current_sensor-1].scratchpad_data, sensors[current_sensor-1].ROM_code);
                measurement_phase = 1; // переходим на другую фазу
                measurement_start_time = msTicks;
            }
        }
        // фаза 1: обрабатываем результат
        else if (measurement_phase == 1) {
						// высчитываем правильность полученой из датчика температуры
            sensors[current_sensor-1].crc8_data = Compute_CRC8(sensors[current_sensor-1].scratchpad_data, 8);
            sensors[current_sensor-1].crc8_data_error = Compute_CRC8(sensors[current_sensor-1].scratchpad_data, 9) == 0 ? 0 : 1;
            
					  // если нет ошибок, сохраняем темпетатуру в нужном формате в структуру 
            if (!sensors[current_sensor-1].crc8_data_error) {
                sensors[current_sensor-1].raw_temp = ((uint16_t)sensors[current_sensor-1].scratchpad_data[1] << 8) | sensors[current_sensor-1].scratchpad_data[0];
                sensors[current_sensor-1].temp = sensors[current_sensor-1].raw_temp * 0.0625;
            }
            
            // Измерение завершено
            measurement_in_progress = 0;
        }
    }
}

// функция для обработки команд, полученных от uart
void ProcessUARTCommand(void) {
    if (uart_string_received) {
        UART_SendString("\r\n"); // Переход на новую строку
        
        // команда help - показать все команды
        if (strcmp((char*)uart_rx_buffer, "help") == 0) {
            UART_SendString("Available commands:");
            UART_SendString("\r\n  help    - Show this message");
            UART_SendString("\r\n  info    - Show sensors info");
            UART_SendString("\r\n  start   - Start measurements");
            UART_SendString("\r\n  stop    - Stop measurements");
            UART_SendString("\r\n  temp    - Show all temperatures");
            UART_SendString("\r\n  temp x  - Show sensor x temperature");
            UART_SendString("\r\n  status  - System status");
						UART_SendString("\r\n  refresh sensors  - Updating the number of sensors when the physical number of connected sensors changes");
        }
        
        // команда info - информация о датчиках
        else if (strcmp((char*)uart_rx_buffer, "info") == 0) {
            UART_SendString("=== SENSORS INFORMATION ===");
            UART_SendString("\r\nFound sensors: ");
            UART_SendNumber(devCount);
            
            for (uint8_t i = 0; i < devCount; i++) {
                UART_SendString("\r\n\r\nSensor ");
                UART_SendNumber(i+1);
                UART_SendString(":");
                UART_SendString("\r\n  ROM: ");
                // Выводим первые 4 байта ROM для идентификации
                for (uint8_t j = 0; j < 4; j++) {
                    UART_SendNumber(sensors[i].ROM_code[j]);
                    if (j < 3) UART_SendChar('.');
                }
                UART_SendString("\r\n  Status: ");
                UART_SendString(sensors[i].is_present ? "PRESENT" : "MISSING");
                
                if (sensors[i].is_present) {
                    UART_SendString("\r\n  Settings: Default (Th=100°C, Tl=-30°C, 9-bit)");
                }
            }
        }
        
        // команда start - начать измерения
        else if (strcmp((char*)uart_rx_buffer, "start") == 0) {
            auto_measurement_enabled = 1;
            UART_SendString("Measurements STARTED");
        }
        
        // команда stop - остановить измерения
        else if (strcmp((char*)uart_rx_buffer, "stop") == 0) {
            auto_measurement_enabled = 0;
            measurement_in_progress = 0; // Сбрасываем текущее измерение
            UART_SendString("Measurements STOPPED");
        }
        
        // команда temp - показать температуры
        else if (strncmp((char*)uart_rx_buffer, "temp", 4) == 0) {
            // Если команда "temp" без параметров
            if (strlen((char*)uart_rx_buffer) == 4) {
                UART_SendString("Temperatures:");
                for (uint8_t i = 0; i < devCount; i++) {
                    PrintSensorTemperature(i);
                }
            }
						// сделать так, чтобы можно было ввести любой параметр...
            // Если команда "temp 1" или "temp 2"
            else if (strlen((char*)uart_rx_buffer) == 6) {
                uint8_t sensor_num = uart_rx_buffer[5] - '0'; // Получаем цифру
                if (sensor_num >= 1 && sensor_num <= devCount) {
                    PrintSensorTemperature(sensor_num - 1);
                } else {
                    UART_SendString("Error: Invalid sensor number");
                }
            }
        }
        
        // команда status - статус системы
        else if (strcmp((char*)uart_rx_buffer, "status") == 0) {
            UART_SendString("=== SYSTEM STATUS ===");
            UART_SendString("\r\nMeasurements: ");
            UART_SendString(auto_measurement_enabled ? "RUNNING" : "STOPPED");
            UART_SendString("\r\nSensors online: ");
            UART_SendNumber(GetOnlineSensorsCount());
            UART_SendString("/");
            UART_SendNumber(devCount);
            UART_SendString("\r\nSystem alert: ");
            UART_SendString(system_alert ? "YES" : "NO");
        }
				
				// команда refresh sensors - если человек подключил сенсоры, то заново начнёт поиск
        else if (strcmp((char*)uart_rx_buffer, "refresh sensors") == 0) {
            while (ds18b20_Reset());
							devCount = Search_ROM(0xF0, &sensors);
					
						UART_SendString("\r\nDS18B20 sensors detected: ");
						UART_SendNumber(devCount); 
        }
        
        // неизвестная команда
        else {
            UART_SendString("Unknown command: ");
            UART_SendString((char*)uart_rx_buffer);
            UART_SendString("\r\nType 'help' for available commands");
        }
        
        uart_string_received = 0;
        uart_rx_index = 0;
        UART_SendPrompt();
    }
}



// Чтение настроек датчика из его памяти
void ReadSensorSettings(uint8_t sensor_index) {
    if (sensor_index >= devCount) return;
    
    if (sensors[sensor_index].is_present) {
        // Читаем скретчпад датчика
        ds18b20_ReadStratchpad(1, sensors[sensor_index].scratchpad_data, sensors[sensor_index].ROM_code);
        
        // Настройки хранятся в байтах 2, 3, 4 скретчпада
        // Байт 2: Th (верхний порог тревоги)
        // Байт 3: Tl (нижний порог тревоги) 
        // Байт 4: Configuration (разрешение)
        
        sensors[sensor_index].scratchpad_data[2]; // Th
        sensors[sensor_index].scratchpad_data[3]; // Tl
        sensors[sensor_index].scratchpad_data[4]; // Configuration
    }
}



//              (перенести в отдельный модуль)
// ==================== работа с UART =========================


int main(void) {			
    uint32_t last_measurement = 0;
		
		// для нескольких попыток подключения
		uint8_t attempt_count = 0;
  
	  // настройка системы
    SystemCoreClockConfigure();
    SysTick_Config(SystemCoreClock / 1000000);
    
    ds18b20_PortInit();
    UART_Init();
	
		// Приветственное сообщение UART
		UART_SendString("\r\n=== TEMPERATURE MONITORING SYSTEM ===");
    
		// находим кол-во датчиков ТОЛЬКО 1 раз, больше их количество нельзя изменить! (пока что)
    while (ds18b20_Reset());
			devCount = Search_ROM(0xF0, &sensors);
	
		UART_SendString("\r\nDS18B20 sensors detected: ");
    UART_SendNumber(devCount);
		
  	 if (devCount > 0) {
        UART_SendString("\r\nWaiting for sensor connection...");
    } 
		
//    if (devCount > 0 && !sensors[0].crc8_data_error) {
//        ds18b20_Init_Settings(1, sensors[0].ROM_code, 0x64, 0xFF9E, 0x1F);
//    }
//    if (devCount > 1 && !sensors[1].crc8_data_error) {
//        ds18b20_Init_Settings(1, sensors[1].ROM_code, 0x32, 0xFC90, 0x7F);
//    }
    
    SensorMonitor_Init();
       
    while (1) {
        // UART получение команды(выполняется всегда)
        UART_ProcessReception();
			   // если система не инициализированна не обрабатываем команды
         if (system_initialized) {
					 // обработка команд
            ProcessUARTCommand();
        }
				 else{
					 	CheckFirstConnection(&system_initialized); 
				 }
        // Мониторинг
        SensorMonitor_Update();
				
        // Измерения
        MeasureTemperature();
        
        // короткая задержка
        DelayMicro(1000);
    }
}