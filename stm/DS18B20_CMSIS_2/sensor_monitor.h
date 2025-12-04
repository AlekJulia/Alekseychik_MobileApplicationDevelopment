#ifndef SENSOR_MONITOR_H
#define SENSOR_MONITOR_H

#include "ds18b20.h" // для доступа к структуре sensor

#ifndef MAX_SENSORS
#define MAX_SENSORS 2
#endif

#define PRESENCE_CHECK_INTERVAL   2000000  // 2 секунды между проверками
#define MAX_MISSING_COUNT         3        // 3 пропуска до сигнала

// флаги состояния системы
extern volatile uint8_t system_alert;      // сигнал об ошибке
extern volatile uint8_t sensors_online;    // количество работающих датчиков
extern volatile uint8_t all_sensors_dead;  // нет подключённых датчиков

// инициализация мониторинга
void SensorMonitor_Init(void);

// проверка первого подключения датчиков
void CheckFirstConnection(uint8_t* system_initialized);

// основная функция обновления мониторинга
void SensorMonitor_Update(void);

// проверка присутствия конкретного датчика
uint8_t SensorMonitor_CheckPresence(uint8_t* rom_code);

// сигнализация об отсутствии датчика
void SensorMissingAlert(uint8_t sensor_index);

// получить количество работающих датчиков
uint8_t GetOnlineSensorsCount(void);

#endif // SENSOR_MONITOR_H