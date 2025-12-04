#include "ds18b20.h"

// volatile - чтобы не оптимизир. чтение, запись так как меняется в прерывании
volatile uint32_t msTicks; 

uint8_t onewire_enum_fork_bit; 

// прерывание системного таймера
void SysTick_Handler(void) {
  msTicks++; // увеличение глобального счётчика
}

// функция задержки в микросекундах
void DelayMicro (uint32_t dlyTicks) {// ожидание заданного кол-во микросек
  uint32_t curTicks;

  curTicks = msTicks;// сохранение начальной точки отсчёта
	// текущ. знач. - сохр. знач. = кол-во прошедш. врем. < сколько нужно ждать
  while ((msTicks - curTicks) < dlyTicks) { __NOP(); }// процессор занят выполнением __NOP() и проверкой условия
}

// настраиваем PB11 как open-drain
void ds18b20_PortInit(void)// настройка gpio под 1-wire
{
	// макросы  GPIO_CRH_MODE11 устанавливают пин P11
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;	// включение тактирования GPIOB // I/O Port B clock enabled, RCC->APB2ENR |= 1000b, IOPBEN = 1;
  GPIOB->CRH |= GPIO_CRH_MODE11;	// crh для pb8-pb15	// mode 11 = выход 50 МГц			// Output Mode, max speed 50MHz, GPIOB->CRH |=   11000000000000b, MODE11 = 11
  GPIOB->CRH |= GPIO_CRH_CNF11_0;	// cnf 01 = выход open drain			// General purpose output Open-drain, GPIOB->CRH |=  100000000000000b, CNF11 = 01
  GPIOB->CRH &= ~GPIO_CRH_CNF11_1;		// General purpose output Open-drain, GPIOB->CRH &= 0111111111111111b, CNF11 = 01
}
//--------------------------------------------------

uint8_t ds18b20_Reset(void)
{// тут всё на макросах для PB11
  uint16_t status;
	GPIOB->BSRR = GPIO_BSRR_BR11;					// set 0 to PIN11  // устанавливаем линию в 0
  DelayMicro(480);											// delay 480 mcs   // мастер держит линию 480 мкс
  GPIOB->BSRR = GPIO_BSRR_BS11;					// set 1 to PIN11  // отпускаем линию становится 1 - так как open-drain
  DelayMicro(60);												// delay 60 mcs / ждём 60 мкс
  status = GPIOB->IDR & GPIO_IDR_IDR11; // читаем линию 	// check value on PIN11, if not zero then device not responds
  DelayMicro(480);	// смотрим ответил ли датчик, будет 0 если датчик есть, так как open-drain										// delay 480 mcs
  return (status ? 1 : 0);
}
//----------------------------------------------------------
// считывание 1 бита данных от датчика
uint8_t ds18b20_ReadBit(void) // выбираем пин В11
{
  uint8_t bit = 0;
  GPIOB->BSRR = GPIO_BSRR_BR11;							 	 // set 0 to PIN11
  DelayMicro(1);															 // delay 1 mcs
	GPIOB->BSRR = GPIO_BSRR_BS11;								 // set 1 to PIN11 
	DelayMicro(14);															 // delay 14 mcs
	bit = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0); // check value on PIN11
	DelayMicro(45);															 // delay 45 mcs
  return bit;
}
//-----------------------------------------------
// считывания 1 байт от датчика
uint8_t ds18b20_ReadByte(void)
{
  uint8_t data = 0;
  for (uint8_t i = 0; i <= 7; i++)
	
	// побитовый сдвиг
	// data = 00000000 при заполнении устанавливаем в позицию i (с конца) считаный бит с датчика
	data |= ds18b20_ReadBit() << i;			// read bit by bit from sensor
  return data;
}
//-----------------------------------------------
// запись одного бита
void ds18b20_WriteBit(uint8_t bit)
{
  GPIOB->BSRR = GPIO_BSRR_BR11;	// датчик определяет бит по длительности низкого уровня				// set 0 to PIN11
  DelayMicro(bit ? 1 : 60);		// чтобы записать 1 держим ноль 1 мкс				// if bit is 1 delay 1mcs else 60mcs
  GPIOB->BSRR = GPIO_BSRR_BS11;					// set 1 to PIN11 
  DelayMicro(bit ? 60 : 1);		// дополнительная задержка для восстановления линии и подготовки к сследующему биту		// if bit is 1 delay 60mcs else 1mcs
}
//-----------------------------------------------
//  запись одного байта
void ds18b20_WriteByte(uint8_t data)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    ds18b20_WriteBit(data >> i & 1); 			// write bit by bit to sendor
    DelayMicro(5);												// delay for protection
  }
}
//-----------------------------------------------
//выбор конкретного датчика
void ds18b20_MatchRom(uint8_t* address)
{
		uint8_t i;
		ds18b20_Reset();
		ds18b20_WriteByte(MATCH_CODE);	// команда выбора по адресу			// send match rom command to sensor
		for(i=0;i<8;i++)
		{
			ds18b20_WriteByte(address[i]); // отправка rom код			// send address to match with sensor address
		}
}

// Scratchpad - энергонезависимая область памяти датчика


// инициализация reset, skip/match, запись параметров Th Tl
void ds18b20_Init(uint8_t mode, uint8_t* address)
{
	uint8_t i;
	uint8_t rom_data [8] = {0};
	ds18b20_Reset();
  if(mode == 0)					//if skip rom mode selected
  {
		ds18b20_WriteByte(SKIP_ROM); 	// работать со всеми				//send skip ROM command
  } 
	else 
	{
		ds18b20_MatchRom(address); 		// работать с одним					//send match code command with address
	}
	ds18b20_WriteByte(WRITE_SCRATCHPAD);  // записать "Scratchpad" 	//send write scratchpad command
	ds18b20_WriteByte(0x64); 		//send Th = 100 максимальная температура						//send Th = 100							//send Th = 101
	ds18b20_WriteByte(0xFF9E); 	//send Tl = -30 минимальная температура						//send Tl = -30
	
//	ds18b20_WriteByte(RESOLUTION_12BIT);  	//set resolution 12 bit
	ds18b20_WriteByte(RESOLUTION_9BIT);  	//set resolution 9 bit
	
}


//-----------------------------------------------
// Дополнительная команда для установки значений в датчике
void ds18b20_Init_Settings(uint8_t mode, uint8_t* address, uint8_t Th, uint8_t Tl, uint8_t resolution)
{
	uint8_t i;
	uint8_t rom_data [8] = {0};
	ds18b20_Reset();
  if(mode == 0)					//if skip rom mode selected
  {
		ds18b20_WriteByte(SKIP_ROM); 					//send skip ROM command
  } 
	else 
	{
		ds18b20_MatchRom(address); 						//send match code command with address
	}
	ds18b20_WriteByte(WRITE_SCRATCHPAD);  	//send write scratchpad command
	//ds18b20_WriteByte(0x64); 								//send Th = 100
	//ds18b20_WriteByte(0xFF9E); 							//send Tl = -30
	
//	ds18b20_WriteByte(RESOLUTION_12BIT);  	//set resolution 12 bit
	
	ds18b20_WriteByte(Th); //send Th = 100
	ds18b20_WriteByte(Tl); //send Tl = -30
	ds18b20_WriteByte(resolution);  	//set resolution 9 bit
	
}
//----------------------------------------------------------

// запуск измерения температуры
void ds18b20_ConvertTemp(uint8_t mode, uint8_t* address)
{
  ds18b20_Reset();
  if(mode == 0) 													//if skip rom mode selected
  {
    ds18b20_WriteByte(SKIP_ROM); 					//send skip ROM command
  } 
	else 
	{
		ds18b20_MatchRom(address); 						//send match code command with address
	}
  ds18b20_WriteByte(CONVERT_TEMP); 				//send convert temp command
}
//----------------------------------------------------------
// чтение 9 байт из внутренней памяти датчика
void ds18b20_ReadStratchpad(uint8_t mode, uint8_t *Data, uint8_t* address)
{
  uint8_t i;
  ds18b20_Reset();
  if(mode == 0)
  {
    ds18b20_WriteByte(SKIP_ROM); 					//if skip rom mode selected
  } 
	else 
	{
		ds18b20_MatchRom(address); 						//send match code command with address
	}
  ds18b20_WriteByte(READ_SCRATCHPAD); 		//send read scratchpad command
  for(i = 0;i < 9; i++)// считывание scratchpad[0] = temp LSB, [1] = temp MSB, [2] = Th, [3] = Tl, [4] = config, [5..6] = reserved, [7] = reserved, [8] = CRC
  {
    Data[i] = ds18b20_ReadByte();					//read scratchpad byte by byte
  }
}

// чтение rom кода при одном устройстве на шине
void ds18b20_ReadROM (uint8_t *Data) {
		uint8_t i;
		ds18b20_Reset();
		ds18b20_WriteByte(READ_ROM);					//send read rom command 		
		for(i = 0;i < 8; i++)
		{
			Data[i] = ds18b20_ReadByte(); 			//read rom byte by byte
		}
}
// вычисление полинома для проверки данных или rom
uint8_t Compute_CRC8 (uint8_t* data, uint8_t length) {
		uint8_t polynomial = 0x8C, crc = 0x0, i = 0, j = 0, lsb = 0, inbyte = 0;
		while (length--) {
				inbyte = data[j];
        for (i = 0; i < 8; i++) {
            lsb = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (lsb) 
							crc ^= polynomial;
            inbyte >>= 1;
        }
				j++;
		}
		return crc; 
}
// ROM Search Routine //
// Нахождение всех датчиков на шине, чтобы потом обращатся к каждому по rom
// OneWire алгоритм
                   // тип команды   // указатель на массив структур, куда будут записаны rom коды
uint8_t Search_ROM(char command, Sensor *sensors) {
    uint8_t i = 0, sensor_num = 0; // кол-во найденныйх датчиков
		char DS1820_done_flag = 0; // флаг завершения поиска
    int DS1820_last_descrepancy = 0; // позиция разветвления
    char DS1820_search_ROM[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // найденный код
    
    int descrepancy_marker, ROM_bit_index; // переменные для текущего поиска
    char return_value, Bit_A, Bit_B; // ответ от датчиков, два бита
    char byte_counter, bit_mask;
 
    return_value = 0; // пока что ничего ненайдено 0
    while (!DS1820_done_flag) {
					// проверка ответа от датчика
					if (ds18b20_Reset()) { // сброс шины, ожидание присутствия 
            return 0; // выходим, если ни один датчик не ответил
        } else { 
            ROM_bit_index=1; // начинаем с первого бита rom
            descrepancy_marker=0; // ещё нет разветвлений
					
            char command_shift = command; // копия команды
					
            for (int n=0; n<8; n++) { // отправка команды поиска по одному биту          // Search ROM command or Search Alarm command
								ds18b20_WriteBit(command_shift & 0x01); // отправляем младший бид
                command_shift = command_shift >> 1; // сдвигаем вправо now the next bit is in the least sig bit position.
            } 
            byte_counter = 0; // начинаем с первого байта rom
            bit_mask = 0x01; // это первый бит байта
						
						// проходим по всем 64 битам rom
            while (ROM_bit_index<=64) {
                Bit_A = ds18b20_ReadBit(); // читаем первый бит
                Bit_B = ds18b20_ReadBit(); // читаем второй бит
							// если оба бита равны 1 - ошибка 
                if (Bit_A & Bit_B) {
                    descrepancy_marker = 0; // data read error, this should never happen
                    ROM_bit_index = 0xFF;
                } else {
										// если один из битов равен 1, другой 0 — на шине нет конфликт
                    if (Bit_A | Bit_B) {
                        // Set ROM bit to Bit_A
                        if (Bit_A) { // ставим бит 1
                            DS1820_search_ROM[byte_counter] = DS1820_search_ROM[byte_counter] | bit_mask; // Set ROM bit to one
                        } else { // ставим бит 0
                            DS1820_search_ROM[byte_counter] = DS1820_search_ROM[byte_counter] & ~bit_mask; // Set ROM bit to zero
                        }
												// оба бита 0 — значит, у разных датчиков разные значения на этом бите
                        // выбираем по какому пути идти (0 или 1)
                    } else {
                        // both bits A and B are low, so there are two or more devices present
											
												// если уже ранее были на этом разветвлении — идём по другой ветке
                        if ( ROM_bit_index == DS1820_last_descrepancy ) { // выбираем 1
                            DS1820_search_ROM[byte_counter] = DS1820_search_ROM[byte_counter] | bit_mask; // Set ROM bit to one
												
													// ecли ещё не дошли до последнего разветвления — идём по 0
                        } else {
                            if ( ROM_bit_index > DS1820_last_descrepancy ) {// выбираем 0
                                DS1820_search_ROM[byte_counter] = DS1820_search_ROM[byte_counter] & ~bit_mask; // Set ROM bit to zero
                                descrepancy_marker = ROM_bit_index;// запоминаем новую точку разветвления
                             // eсли предыдущий ROM бит = 0, запоминаем позицию как новую точку ветвления
														} else {
                                if (( DS1820_search_ROM[byte_counter] & bit_mask) == 0x00 )
                                    descrepancy_marker = ROM_bit_index;
                            }
                        }
                    }
										// отправляем выбранный бит обратно в шину, чтобы датчики знали, куда движемся
										ds18b20_WriteBit(DS1820_search_ROM[byte_counter] & bit_mask);
                    ROM_bit_index++;// переходим к следующему биту rom
										
										// eсли прошли все 8 бит текущего байта
                    if (bit_mask & 0x80) {
                        byte_counter++; // переходим к следующему байту rom
                        bit_mask = 0x01; // снова начинаем с младшего бита
                    } else {
                        bit_mask = bit_mask << 1;// сдвигаем маску влево на 1 (выбираем следующий бит)
                    }
                }
            }
						 // сохраняем последнее место, где было ветвление
            DS1820_last_descrepancy = descrepancy_marker;
						
						 // копируем найденный ROM в структуру sensors[sensor_num]
						for (i = 0; i < 8; i++) {
							 sensors[sensor_num].ROM_code[i] = DS1820_search_ROM[i];
						}
						sensor_num++; // увеличиваем счётчик найденных устройств
        }
				 // если больше нет точек разветвления — все устройства найдены
        if (DS1820_last_descrepancy == 0)
            DS1820_done_flag = 1;
    }
    return sensor_num; // возвращаем количество найденных датчиков
}



