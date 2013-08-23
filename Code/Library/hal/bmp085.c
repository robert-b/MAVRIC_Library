/*
 * bmp085.c
 *
 * Created: 12/03/2013 22:14:46
 *  Author: sfx
 */ 
#include "bmp085.h"

#include "twim.h"
#include "delay.h"

#include "math.h"
#include "time_keeper.h"
#include <stdbool.h>

pressure_data pressure_outputs;

int16_t bmp085_read_int(unsigned char address) {
	int16_t result;
	twim_write(&AVR32_TWIM0, (uint8_t*) &address, 1, BMP085_SLAVE_ADDRESS, false);
	twim_read(&AVR32_TWIM0, (uint8_t*)&(result), 2, BMP085_SLAVE_ADDRESS, false);
	return result;
}

void init_bmp085(){
	init_bmp085_slow();
}


  int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
  uint16_t ac4, ac5, ac6;


void init_bmp085_slow(){
	
	ac6 = 23153;
	ac5 = 32757;
	mc = -8711;
	md = 2868;
	b1 = 6190;
	b2 = 4;
	ac3 = -14383;
	ac2 = -72;
	ac1 = 408;
	ac4 = 32741;

	
	ac1=bmp085_read_int(0xAA);
	ac2=bmp085_read_int(0xAC);
	ac3=bmp085_read_int(0xAE);
	ac4=bmp085_read_int(0xB0);
	ac5=bmp085_read_int(0xB2);
	ac6=bmp085_read_int(0xB4);
	b1=bmp085_read_int(0xB6);
	b2=bmp085_read_int(0xB8);
	mb=bmp085_read_int(0xBA);
	mc=bmp085_read_int(0xBC);
	md=bmp085_read_int(0xBE);
	
	
}

 


pressure_data* get_pressure_data_slow() {
		
		int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
		uint32_t B4, B7;
			
		uint8_t start_address;
		
		uint8_t start_command_temp []={BMP085_CONTROL, BMP085_READTEMPCMD};
		uint8_t start_command_pressure []={BMP085_CONTROL, BMP085_READPRESSURECMD+ (oversampling << 6)};
		int32_t sealevelPressure=101325;
		
		// calibration: use datasheet numbers!

		switch (pressure_outputs.state) {
		case IDLE:
			twim_write(&AVR32_TWIM0, (uint8_t*) &start_command_temp, 2, BMP085_SLAVE_ADDRESS, false);
		    //delay_ms(7);
			pressure_outputs.state=GET_TEMP;
			break;
		case GET_TEMP:
			start_address=BMP085_TEMPDATA;
			twim_write(&AVR32_TWIM0, (uint8_t*) &start_address, 1, BMP085_SLAVE_ADDRESS, false);
			twim_read(&AVR32_TWIM0, (uint8_t*)&(pressure_outputs.raw_temperature), 2, BMP085_SLAVE_ADDRESS, false);
		
			twim_write(&AVR32_TWIM0, (uint8_t*) &start_command_pressure, 2, BMP085_SLAVE_ADDRESS, false);
			pressure_outputs.state=GET_PRESSURE;
			//delay_ms(15);
			break;
		case GET_PRESSURE:
			start_address=BMP085_PRESSUREDATA;
			twim_write(&AVR32_TWIM0, (uint8_t*) &start_address, 1, BMP085_SLAVE_ADDRESS, false);
			twim_read(&AVR32_TWIM0, (uint8_t*)&(pressure_outputs.raw_pressure), 3, BMP085_SLAVE_ADDRESS, false);
		
			UP= ((uint32_t)pressure_outputs.raw_pressure[0]<<16 |(uint32_t)pressure_outputs.raw_pressure[1]<<8 | (uint32_t)pressure_outputs.raw_pressure[2]) >> (8-oversampling);
 
			UT=pressure_outputs.raw_temperature[0]<<8 |pressure_outputs.raw_temperature[1];
			// step 1
			X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) / pow(2,15);
			X2 = ((int32_t)mc * pow(2,11)) / (X1+(int32_t)md);
			B5 = X1 + X2;
			pressure_outputs.temperature = (B5+8)/pow(2,4);
			pressure_outputs.temperature /= 10;
	
			// do pressure calcs
			B6 = B5 - 4000;
			X1 = ((int32_t)b2 * ( (B6 * B6)>>12 )) >> 11;
			X2 = ((int32_t)ac2 * B6) >> 11;
			X3 = X1 + X2;
			B3 = ((((int32_t)ac1*4 + X3) << oversampling) + 2) / 4;


			X1 = ((int32_t)ac3 * B6) >> 13;
			X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
			X3 = ((X1 + X2) + 2) >> 2;
			B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
			B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> oversampling );


			if (B7 < 0x80000000) {
				p = (B7 * 2) / B4;
			} else {
				p = (B7 / B4) * 2;
			}
			X1 = (p >> 8) * (p >> 8);
			X1 = (X1 * 3038) >> 16;
			X2 = (-7357 * p) >> 16;

			p = p + ((X1 + X2 + (int32_t)3791)>>4);

			pressure_outputs.pressure=p;
		
			pressure_outputs.altitude = 44330 * (1.0 - pow(pressure_outputs.pressure /sealevelPressure,0.190295));
			pressure_outputs.last_update=get_micros();
			pressure_outputs.state=IDLE;
			break;
			
		}
		pressure_outputs.last_state_update=get_micros();
		return &pressure_outputs;
}

bool newValidBarometer()
{
	uint32_t tnow = get_micros();
	if ((tnow - pressure_outputs.last_update) < 1000) // 1 millisecond
	{
		return true;
	}else{
		return false;
	}
	
}