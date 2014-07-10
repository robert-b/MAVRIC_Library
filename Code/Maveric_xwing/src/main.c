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

#include "time_keeper.h"
#include "streams.h"

#include "bmp085.h"

#include "scheduler.h"
#include "central_data.h"
#include "boardsupport.h"
#include "mavlink_waypoint_handler.h"
#include "navigation.h"
#include "tasks.h"
#include "neighbor_selection.h"
#include "orca.h"
//#include "flashvault.h"

central_data_t *centralData;

void initialisation() {
	int i;
	enum GPS_Engine_Setting engine_nav_settings = GPS_ENGINE_AIRBORNE_4G;

	centralData = central_data_get_pointer_to_struct();
	boardsupport_init(centralData);
	central_data_init();
	

	radar_module_init();
	print_util_dbg_print("Debug stream initialised\n");

	//gps_ublox_init(engine_nav_settings);
	
	servo_pwm_set(&servo_failsafe);

	onboard_parameters_init();
	mavlink_actions_init();
	position_estimation_init(&centralData->position_estimator, &centralData->pressure, &centralData->GPS_data);
	
	centralData->imu1.attitude.calibration_level=LEVELING;	
	centralData->mav_state = MAV_STATE_CALIBRATING;
	centralData->mav_mode = MAV_MODE_PREFLIGHT;

//	imu_calibrate_gyros(&centralData->imu1);
	for (i=400; i>0; i--) {
		imu_get_raw_data(&(centralData->imu1));
		imu_update(&(centralData->imu1), &centralData->position_estimator, &centralData->pressure, &centralData->GPS_data);	
		mavlink_schedule_update();	
		delay_ms(5);
	}
	// after initial leveling, initialise accelerometer biases
	
	/*
	centralData->imu1.attitude.calibration_level=LEVEL_PLUS_ACCEL;
	for (i=100; i>0; i--) {
		imu_update(&(centralData->imu1), &centralData->position_estimator, &centralData->pressure, &centralData->GPS_data);	
		mavlink_schedule_update();			
		delay_ms(5);
	}*/
	centralData->imu1.attitude.calibration_level=OFF;
	//reset position estimate
	for (i=0; i<3; i++) {
		// clean acceleration estimate without gravity:
		centralData->position_estimator.vel_bf[i]=0.0;
		centralData->position_estimator.vel[i]=0.0;
		centralData->position_estimator.localPosition.pos[i]=0.0;
	}
	centralData->mav_state = MAV_STATE_STANDBY;
	centralData->mav_mode = MAV_MODE_MANUAL_DISARMED;
	navigation_init();
	waypoint_handler_init();
	//e_init();
	
	neighbors_selection_init();
	orca_init();
	
	LED_On(LED1);
}



void main (void)
{
	
	initialisation();
	
	create_tasks();
	
	// turn on simulation mode: 1: simulation mode, 0: reality
	centralData->simulation_mode = 0;

	while (1==1) {
		
		//scheduler_update(tasks_get_main_taskset(), FIXED_PRIORITY);
		scheduler_update(tasks_get_main_taskset(), ROUND_ROBIN);
		
		//LED_On(LED1);
	}		
}


