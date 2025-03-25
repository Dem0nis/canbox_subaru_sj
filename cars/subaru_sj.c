static void subaru_sj_ms_wheel_angle(const uint8_t *msg, struct msg_desc_t *desc) {
    if (is_timeout(desc)) {
        carstate.wheel = 0;
        return;
    }

    // Объединяем байты в 16-битное значение
    int16_t angle = (int16_t)((msg[0] << 8) | msg[1]);

    // Масштабируем угол в диапазон [-100%, 100%]
    // Максимальное значение для 2.5 оборотов: 5887 (0x16FF,) Минимальное для 2.5 оборотов: -5888 (0xEA00)
    carstate.wheel = scale(angle, -5888, 5887, -100, 100); 
}

//=====================================================================================

static void subaru_sj_ms_speed(const uint8_t * msg, struct msg_desc_t * desc)
{
	if (is_timeout(desc)) {
		carstate.speed = 0;
		return;
	}
	carstate.speed = (((uint32_t)msg[1] & 0x0F) << 8 | (uint32_t)msg[0]) * 0.05625 + 0.5;
}

//=====================================================================================

static void subaru_sj_ms_tacho(const uint8_t * msg, struct msg_desc_t * desc)
{
	if (is_timeout(desc)) {
		carstate.taho = 0;
		carstate.engine = STATE_UNDEF;
		return;
	}

        carstate.taho = ((msg[5] & 0x0F) << 8) | msg[4];

	if (carstate.taho > 600)
		carstate.engine = 1;
	else
		carstate.engine = 0;
}

//=====================================================================================

static void subaru_sj_ms_voltage(const uint8_t * msg, struct msg_desc_t * desc)
{
	if (is_timeout(desc)) {
		carstate.voltage = 0;
		return;
	}

        carstate.voltage = (msg[6] / 100) + 10;
//	carstate.voltage = 5 + (0.05 * msg[6]);
}

//=====================================================================================

static void subaru_sj_ms_gear(const uint8_t * msg, struct msg_desc_t * desc)
{
    if (is_timeout(desc)) {
        carstate.selector = STATE_UNDEF;
        return;
    }

    switch (msg[6]) {
        case 0xFF:  // Паркинг (P)
            carstate.selector = e_selector_p;
            break;
        case 0xEE:  // Задний ход (R)
            carstate.selector = e_selector_r;
            break;
        case 0x00:  // Нейтраль (N)
            carstate.selector = e_selector_n;
            break;
        case 0x11:  // Драйв (D) или Ручной режим (M)
            if (msg[7] == 0x01)
                carstate.selector = e_selector_d;
            else
                carstate.selector = e_selector_m;
            break;
        default:
            carstate.selector = STATE_UNDEF;
            break;
    }
}

//=====================================================================================

static void subaru_sj_ms_light(const uint8_t * msg, struct msg_desc_t * desc)
{
	if (is_timeout(desc)) {
		carstate.acc = STATE_UNDEF;
		carstate.ign = STATE_UNDEF;
		carstate.near_lights = STATE_UNDEF;
		carstate.park_lights = STATE_UNDEF;
		carstate.park_break  = STATE_UNDEF;
		return;
	}
	
	carstate.acc            = (msg[0] & 0x20) ? 1:0;
	carstate.ign 		= (msg[0] & 0x40) ? 1:0;
	carstate.near_lights 	= (msg[7] & 0x08) ? 1:0;
	carstate.park_lights	= (msg[7] & 0x04) ? 1:0;
	carstate.park_break	= (msg[6] & 0x08) ? 1:0;
}

//=====================================================================================
/*
static void subaru_sj_ms_fuel(const uint8_t * msg, struct msg_desc_t * desc)
{
	if (is_timeout(desc)) {
		carstate.fuel_lvl = STATE_UNDEF;
		return;
	}
	
	carstate.fuel_lvl = (msg[7] / 32) * 60;
}
*/
//=====================================================================================

static void subaru_sj_ms_climate(const uint8_t * msg, struct msg_desc_t * desc)
{
	if (is_timeout(desc)) {

		car_air_state.ac   = STATE_UNDEF;
		car_air_state.rear = STATE_UNDEF;
                car_air_state.recycling = STATE_UNDEF;
		car_air_state.powerfull = STATE_UNDEF;
                car_air_state.ac_max = STATE_UNDEF;
		car_air_state.dual = STATE_UNDEF;
		car_air_state.wind = STATE_UNDEF;
		car_air_state.middle = STATE_UNDEF;
		car_air_state.floor = STATE_UNDEF;
		car_air_state.fanspeed = STATE_UNDEF;
		car_air_state.l_temp = STATE_UNDEF;
		car_air_state.r_temp = STATE_UNDEF;
		return;
	}

	car_air_state.ac = (msg[1] & 0x10) ? 1 : 0;
	car_air_state.rear = (msg[1] & 0x20) ? 1 : 0;
	car_air_state.rear_lock = (msg[1] & 0x20) ? 1 : 0; // найти бит активности!
	car_air_state.recycling = (msg[1] & 0x40) ? 1 : 0;
	car_air_state.dual = (msg[1] & 0x02) ? 1 : 0;
//	car_air_state.powerfull = (msg[1] & 0x04) ? 1 : 0;
	car_air_state.ac_max = (msg[1] & 0x04) ? 1 : 0;
        car_air_state.wind = (msg[0] & 0x10) ? 1 : 0; // было 0x04
        car_air_state.middle = (msg[0] & 0x08) ? 1 : 0; 
        car_air_state.floor = (msg[0] & 0x0C) ? 1 : 0;

        car_air_state.fanspeed = (msg[0] & 0xE0) >> 5; // Cкорость вентилятора 
	
		if (msg[2] == 0x00) return 0x00;  // Lo → 0x00
		if (msg[2] == 0xB0) return 0x1F;  // Hi → 0x1F
		if (msg[2] < 0x28 || msg[2] > 0x68) return 0xFF;  // Проверка допустимого диапазона CAN (0x14–0x7C)
	car_air_state.l_temp = (msg[2] >= 0x28 && msg[2] <= 0x68) ? (msg[2] - 36) / 4 : 0xFF; // Преобразование в USART
//			return;
		
		if (msg[3] == 0x00) return 0x00;  
		if (msg[3] == 0xB0) return 0x1F; 
		if (msg[3] < 0x28 || msg[3] > 0x68) return 0xFF;    // Проверка допустимого диапазона CAN (0x14–0x7C)
	car_air_state.r_temp = (msg[3] >= 0x28 && msg[3] <= 0x68) ? (msg[3] - 36) / 4 : 0xFF; // Преобразование в USART
//		if (car_air_state.r_temp > 0x1C) car_air_state.r_temp = 0x11;	// Ограничение максимума USART до 0x11
			return;		
}

//=====================================================================================

static void subaru_sj_ms_belt(const uint8_t * msg, struct msg_desc_t * desc)
{
	if (is_timeout(desc)) {
		carstate.fuel_lvl = STATE_UNDEF;
		carstate.low_fuel_lvl = STATE_UNDEF;
		carstate.ds_belt = STATE_UNDEF;
		return;
	}

	carstate.fuel_lvl = (0x02FF - ((msg[1] & 0x0F) << 8 | msg[0])) * 65.0f / 0x02FF;
//	if ((msg[6] & 0x80) == 0x80)
//		carstate.low_fuel_lvl = 1;
//	else
//	        carstate.low_fuel_lvl = 0;

	carstate.ds_belt    = (msg[5] & 0x01) ? 1:0;
//	carstate.ps_belt    = (msg[5] & 0x02) ? 1:0;
//	carstate.rs_belt    = (msg[5] & 0x03) ? 1:0;
}

//=====================================================================================

static void subaru_sj_ms_temp(const uint8_t * msg, struct msg_desc_t * desc)
{
	if (is_timeout(desc)) {
		carstate.temp = 0;
		return;
	}

        carstate.temp = (msg[3] - 40);
}

//=====================================================================================

static void subaru_sj_ms_door(const uint8_t * msg, struct msg_desc_t * desc)
{
    if (is_timeout(desc)) {
        carstate.fl_door = STATE_UNDEF;
        carstate.fr_door = STATE_UNDEF;
        carstate.rl_door = STATE_UNDEF;
        carstate.rr_door = STATE_UNDEF;
        carstate.tailgate = STATE_UNDEF;
        return;
    }

    carstate.fl_door = (msg[3] & 0x01) ? 1 : 0;  // Передняя левая дверь (бит 0)
    carstate.fr_door = (msg[3] & 0x02) ? 1 : 0;  // Передняя правая дверь (бит 1)
    carstate.rl_door = (msg[3] & 0x04) ? 1 : 0;  // Задняя левая дверь (бит 2)
    carstate.rr_door = (msg[3] & 0x08) ? 1 : 0;  // Задняя правая дверь (бит 3)
    carstate.tailgate = (msg[3] & 0x10) ? 1 : 0; // Багажник (бит 5)
}

//=====================================================================================

static void subaru_sj_ms_illum(const uint8_t * msg, struct msg_desc_t * desc)
{
	if (is_timeout(desc)) {

		carstate.illum = STATE_UNDEF;
		return;
	}

	carstate.illum = scale(msg[0], 0x00, 0xFF, 0, 100);
}

//=====================================================================================

static void subaru_sj_ms_low_washer(const uint8_t * msg, struct msg_desc_t * desc)
{
	if (is_timeout(desc)) {

		carstate.low_washer = STATE_UNDEF;
		return;
	}

	carstate.low_washer = (msg[6] & 0x04) ? 1 : 0;
}

//=====================================================================================

static void subaru_sj_ms_odometr(const uint8_t * msg, struct msg_desc_t * desc)
{

    if (is_timeout(desc)) {
        carstate.odometer = STATE_UNDEF;
        return;
    }

    // Вычисление одометра
    carstate.odometer = (((uint32_t)msg[3] << 24) | ((uint32_t)msg[2] << 16) | ((uint32_t)msg[1] << 8) | (uint32_t)msg[0]) / 10;
}
//=====================================================================================

struct msg_desc_t subaru_sj_ms[] =
{
	{ 0x002,	  23, 0, 0, subaru_sj_ms_wheel_angle}, // B0,B1 Stering Whell Angle
	{ 0x0D1,	  90, 0, 0, subaru_sj_ms_speed}, // B0,B1 Litle endian, B2 Brake pedal
	{ 0x141,	  10, 0, 0, subaru_sj_ms_tacho}, // B0,B1 Tachometr
	{ 0x140,	 100, 0, 0, subaru_sj_ms_voltage}, // B5 inv/	
	{ 0x148,	  30, 0, 0, subaru_sj_ms_gear}, // Gear B6, Transm B7 
	{ 0x152,	 300, 0, 0, subaru_sj_ms_light}, // B6,B7 lights, turnlight, brakes, 
//	{ 0x156,	  30, 0, 0, subaru_sj_ms_fuel},
	{ 0x281,	1000, 0, 0, subaru_sj_ms_climate},
	{ 0x282,	 100, 0, 0, subaru_sj_ms_belt}, // B1 inv. (01, 02, 03)
	{ 0x360,	3000, 0, 0, subaru_sj_ms_temp}, // B1 inv. 
	{ 0x374,	1000, 0, 0, subaru_sj_ms_door}, // B3 Door status,
	{ 0x376,	 100, 0, 0, subaru_sj_ms_illum}, // Illumination
	{ 0x6D0,	1000, 0, 0, subaru_sj_ms_low_washer},
	{ 0x6D1,	2000, 0, 0, subaru_sj_ms_odometr}, // B0,B1,B2,B3 - Odometr.
};
