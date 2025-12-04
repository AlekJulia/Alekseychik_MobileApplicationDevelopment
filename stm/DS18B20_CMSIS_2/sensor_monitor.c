#include "sensor_monitor.h"
#include "stm32f10x.h"
#include "ds18b20.h"
#include "uart_driver.h" 

// объявляем extern для доступа к переменным из main.c
extern volatile uint32_t msTicks;
extern uint8_t devCount; // сколько датчиков найдено
extern Sensor sensors[MAX_SENSORS]; // массив датчиков из main

volatile uint8_t system_alert = 0; // флаг ошибки
volatile uint8_t sensors_online = 0; // кол-во подключённых датчиков
volatile uint8_t all_sensors_dead = 0; // нет подключённых датчиков
static uint32_t last_presence_check = 0; // последняя проверка

// ================= ФУНКЦИИ =================

// обработка ответившего датчика
static uint8_t ProcessRespondingSensor(uint8_t i, uint8_t* need_show_status) {
    sensors[i].missing_counter = 0; // счётчик пропусков
    sensors[i].alert_was_sent = 0; // сбрасываем флаг отправки ошибки
    
    if (!sensors[i].is_present) { // если раньше не отвечал
        sensors[i].is_present = 1; // теперь ответил
        sensors[i].crc8_data_error = 0; // очищение ошибки
        sensors[i].crc8_rom_error = 0;
        sensors[i].raw_temp = 0;
        
        // сообщение о том, что датчик переподключился
        UART_SendString("\r\n[INFO] Sensor ");
        UART_SendNumber(i + 1);
        UART_SendString(" RECONNECTED");
        *need_show_status = 1;
    }
    
    return 1; // датчик онлайн
}

// обработка неответившего датчика
static uint8_t ProcessMissingSensor(uint8_t i, uint8_t* need_show_status) {
   
		if (sensors[i].missing_counter < MAX_MISSING_COUNT){
			sensors[i].missing_counter++; // прибавляем к пропускам
		}
    else {
        if (sensors[i].is_present) { // если раньше работал
            sensors[i].is_present = 0; // теперь не работает
            SensorMissingAlert(i);  // функция ошибки (один раз)
            *need_show_status = 1;
        }
    }
    
    return 0; // датчик офлайн
}

// показать статус онлайн датчиков
static void ShowOnlineStatus(uint8_t current_online) {
    UART_SendString("\r\n[INFO] Sensors online: ");
    UART_SendNumber(current_online); // выводим обновлённое значение датчиков
    UART_SendString("/");
    UART_SendNumber(devCount);
    UART_SendPrompt();
}

// проверить все ли датчики оффлайн
static uint8_t CheckAllSensorsDead(void) {
    for (uint8_t i = 0; i < devCount; i++) {
        if (sensors[i].is_present) { // если хоть один есть
            return 0; // не все оффлайн
        }
    }
    return 1; // все оффлайн
}

// обработка состояния все датчики оффлайн
static void HandleAllSensorsDead(uint8_t all_are_dead, uint8_t* all_dead_alert_sent) {
    if (all_are_dead && devCount > 0) {
        all_sensors_dead = 1;
        system_alert = 1;
        
        if (!(*all_dead_alert_sent)) {
            // отправляем сообщение 1 раз
            UART_SendString("\r\n[CRITICAL] ALL SENSORS OFFLINE");
            UART_SendString("\r\nSystem functions limited");
            UART_SendString("\r\nThe functions will be available if at least one sensor is online");
            UART_SendPrompt();
            *all_dead_alert_sent = 1; 
        }
    } else {
        all_sensors_dead = 0;
        system_alert = 0;
        *all_dead_alert_sent = 0;
    }
}

// основной цикл проверки датчиков
static void ProcessSensorCheckCycle(uint8_t* all_dead_alert_sent) {
    uint8_t current_online = 0; // счётчик онлайн датчиков
    uint8_t need_show_status = 0; // флаг для статуса, который показываем, только когда меняется кол-во датчиков
    
    // проверка каждого датчика
    for (uint8_t i = 0; i < devCount; i++) {
			  // 	вызываем функцию проверки присутствия датчика, если есть, прибавляем к общему кол-ву датчиков
        if (SensorMonitor_CheckPresence(sensors[i].ROM_code)) {
					  // функция проверки, для вызова доп информации, когда датчик переподключился
            current_online += ProcessRespondingSensor(i, &need_show_status);
        } else {
					  // иначе обрабатываем ошибку, чтобы вывести доп информацию
            ProcessMissingSensor(i, &need_show_status);
        }
    }
    
    // сохранение кол-ва онлайн датчиков
    sensors_online = current_online;
    
    // показываем статус если нужно (показывается, когда менялось кол-во датчиков)
    if (need_show_status) {
        ShowOnlineStatus(current_online);
    }
    
    // проверяем нет ли всех датчиков? (is_present == 0 для всех)
    uint8_t all_are_dead = CheckAllSensorsDead();
    
    // обработка состояния системы
    HandleAllSensorsDead(all_are_dead, all_dead_alert_sent);
}


void SensorMonitor_Init(void) {
    system_alert = 0; // нет ошибок
    sensors_online = 0; // 0 датчиков онлайн
    all_sensors_dead = 0;
    last_presence_check = 0; // ещё не проверяли ошибки
}

// функция для вывода информации при первом подключении
void CheckFirstConnection(uint8_t* system_initialized) {
    // static чтобы функция помнила, что уже проверяла
    static uint8_t already_checked = 0;
    
    if (already_checked || *system_initialized || devCount == 0) {
        return;
    }
    
    uint8_t all_connected_now = 1;
    
    for (uint8_t i = 0; i < devCount; i++) {
        if (SensorMonitor_CheckPresence(sensors[i].ROM_code)) {
            if (!sensors[i].is_present) {
                sensors[i].is_present = 1;
                sensors[i].alert_was_sent = 0;
                
                UART_SendString("\r\n[INFO] Sensor ");
                UART_SendNumber(i + 1);
                UART_SendString(" CONNECTED");
            }
        } else {
            all_connected_now = 0;
        }
    }
    
    if (all_connected_now) {
        already_checked = 1;
        *system_initialized = 1;
        
        UART_SendString("\r\nSystem initialized successfully");
        UART_SendString("\r\nType 'help' for commands list");
        UART_SendPrompt();
    }
}

// мониторинг одного датчика
uint8_t SensorMonitor_CheckPresence(uint8_t* rom_code) {
    if (ds18b20_Reset()) {
        return 0; // если датчики не ответили на сброс, значит их нет
    }
    
    ds18b20_MatchRom(rom_code); // зовём датчик по адресу
    ds18b20_WriteByte(READ_SCRATCHPAD); // считываем данные
    uint8_t data_byte = ds18b20_ReadByte(); // читаем ответ
    
    if (data_byte == 0xFF || data_byte == 0x00) {
        return 0; // если ничего нет в ответе, то датчика тоже нет
    }
    
    return 1; // иначе датчик присутствует
}

// сигнализация об отсутствии датчика
void SensorMissingAlert(uint8_t sensor_index) {
    // чтобы отправлять ошибку об отсутствии датчика 1 раз
    if (!sensors[sensor_index].alert_was_sent) {
        system_alert = 1;
        UART_SendString("\r\n[ALERT] Sensor ");
        UART_SendNumber(sensor_index + 1);
        UART_SendString(" is MISSING!");
        
        // ошибка была отправленна
        sensors[sensor_index].alert_was_sent = 1;
    }
}

// получить количество работающих датчиков
uint8_t GetOnlineSensorsCount(void) {
    return sensors_online;
}

// обновление мониторинга
void SensorMonitor_Update(void) {
    static uint8_t initialized = 0; // static переменная помнит своё значение между вызовами функций
    static uint8_t all_dead_alert_sent = 0; // флаг - уже отправляли сообщение нет ни одного датчика
    
    if (!initialized) { // если не инициализирован, инициализируем
        SensorMonitor_Init();
        initialized = 1;
    }
    
    // проверка, сколько прошло с последней проверки (чтобы проверять каждые 2 сек)
    if ((msTicks - last_presence_check) > PRESENCE_CHECK_INTERVAL) {
        last_presence_check = msTicks;
        
			  // главная фцнкция мониторинга
        ProcessSensorCheckCycle(&all_dead_alert_sent);
    }
}