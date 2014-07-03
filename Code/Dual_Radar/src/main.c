/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * AVR Software Framework (ASF).
 */
#include <asf.h>
#include "led.h"
#include "delay.h"
//#include "stdio_serial.h"
#include "print_util.h"
#include "generator.h"

#include "i2c_driver_int.h"
#include "qfilter.h"
#include "stabilisation.h"
#include "streams.h"

#include "bmp085.h"

#include "scheduler.h"
#include "boardsupport.h"
#include "mavlink_actions.h"
#include "doppler_radar.h"
#include "radar_driver.h"
#include "i2c_slave_interface.h"



central_data_t *central_data;

//NEW_TASK_SET(main_tasks, 10)
task_set main_tasks;

// size of the double buffer (processing occurs when half is filled)
#define SAMPLE_BUFFER_SIZE 256


int16_t sample_buffer[4][SAMPLE_BUFFER_SIZE];
int16_t input_buffer[4][RADAR_BUFFER_SIZE];


	

void initialisation() {
	int i;
	
	main_tasks.number_of_tasks=30;
	initialise_board(central_data);
	

	init_radar();

	Enable_global_interrupt();
		
	//dbg_print("Debug stream initialised\n");


	onboard_parameters_init();
	init_mavlink_actions();
	
	
}

void main (void)
{
	int i=0;
	int ch=0;
	int counter=0;
	uint32_t last_looptime, this_looptime;
	int wait_for_buffer=0;
	
	initialisation();
	
	init_scheduler(&main_tasks);
	register_task(&main_tasks, 0, 1000, RUN_REGULAR, &mavlink_stream_protocol_update);
	// main loop
	counter=0;
	// turn on radar power:
	switch_power(1,0);


	Init_ADCI(1500000, ADCIFA_REF1V);
	adc_sequencer_add(&sample_buffer[0], AVR32_ADCIFA_INP_ADCIN0, AVR32_ADCIFA_INN_ADCIN8, ADCIFA_SHG_16);  
	adc_sequencer_add(&sample_buffer[1], AVR32_ADCIFA_INP_ADCIN1, AVR32_ADCIFA_INN_ADCIN8, ADCIFA_SHG_16);  
	adc_sequencer_add(&sample_buffer[2], AVR32_ADCIFA_INP_ADCIN3, AVR32_ADCIFA_INN_ADCIN8, ADCIFA_SHG_16);  
	adc_sequencer_add(&sample_buffer[3], AVR32_ADCIFA_INP_ADCIN4, AVR32_ADCIFA_INN_ADCIN8, ADCIFA_SHG_16); 
	
	LED_On(LED0);
	delay_ms(1000);
	LED_Off(LED0);
	delay_ms(1000);

	
	// start sampling in continuous mode
	ADCI_Start_Sampling(SAMPLE_BUFFER_SIZE, Sampling_frequency, 16, 1, true);
	while (1==1) {
		this_looptime=get_millis();
		
		if (((ADCI_get_sampling_status()>=SAMPLE_BUFFER_SIZE/2) && (wait_for_buffer==0)) ||
		   ((ADCI_get_sampling_status()<SAMPLE_BUFFER_SIZE/2) && (wait_for_buffer==1)))
		 {  // half of the sample buffer is ready for processing
			// copy samples to end of input buffer
			for (ch=0; ch<4; ch++) {
				for (i=0; i<RADAR_BUFFER_SIZE-(SAMPLE_BUFFER_SIZE/2); i++) {
					input_buffer[ch][i]=input_buffer[ch][i+(SAMPLE_BUFFER_SIZE/2)];
				}
				// get new samples
				if (wait_for_buffer==0) {
					for (i=0; i<SAMPLE_BUFFER_SIZE/2; i++) {
						input_buffer[ch][RADAR_BUFFER_SIZE-(SAMPLE_BUFFER_SIZE/2)+i]=sample_buffer[ch][i];
					}
				} else {
					for (i=0; i<SAMPLE_BUFFER_SIZE/2; i++) {
						input_buffer[ch][RADAR_BUFFER_SIZE-(SAMPLE_BUFFER_SIZE/2)+i]=sample_buffer[ch][i+(SAMPLE_BUFFER_SIZE/2)];
					}
				}
			}
			//ADCI_Start_Sampling(SAMPLE_BUFFER_SIZE, Sampling_frequency, 16, 1, true);
			wait_for_buffer=1-wait_for_buffer;
			LED_On(LED1);

			calculate_radar(&input_buffer[0], &input_buffer[1]);
			mavlink_send_radar();
			//mavlink_send_radar_raw();
			
			//dbg_putfloat(get_tracked_target()->velocity,2);
			//dbg_print(".\n");

		}			
		
		//run_scheduler_update(&main_tasks, FIXED_PRIORITY);
		mavlink_stream_protocol_update();
		//mavlink_msg_named_value_float_send(MAVLINK_COMM_0, get_millis(), "ADC_period", get_adc_int_period());

		LED_Off(LED1);

		counter=(counter+1)%1000;
		last_looptime=this_looptime;	
	}		
}


