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
#include "sysclk.h"
#include "sleepmgr.h"
#include "led.h"
#include "delay.h"
//#include "stdio_serial.h"
#include "print_util.h"
#include "generator.h"

#include "time_keeper.h"
#include "i2c_driver_int.h"
#include "qfilter.h"
#include "stabilisation.h"
#include "streams.h"

#include "bmp085.h"

#include "scheduler.h"
#include "boardsupport.h"
#include "mavlink_actions.h"
#include "radar_module_driver.h"

#include "gps_ublox.h"
#include "estimator.h"
#include "navigation.h"

pressure_data *pressure;


board_hardware_t *board;

NEW_TASK_SET(main_tasks, 10)
	
	
task_return_t run_imu_update() {
	imu_update(&(board->imu1));	
}	

void rc_user_channels(uint8_t *chanSwitch, int8_t *rc_check, int8_t *motorbool)
{
	if (getChannel(4)>0 && getChannel(5)>0)
	{
		*chanSwitch |= 0x00;
	}else if(getChannel(4)<0 && getChannel(5)>0){
		*chanSwitch |= 0x01;
	}else if (getChannel(4)<0 && getChannel(5)<0){
		*chanSwitch |= 0x02;
	}else{
		*chanSwitch |= 0x03;
	}
	
	//dbg_print("chanSwitch ");
	//dbg_print_num(*chanSwitch,10);
	//dbg_print_num(getChannel(4),10);
	//dbg_print_num(getChannel(5),10);
	//dbg_print("\n");
	
	if(((getChannel(S_THROTTLE)/350.0)<-0.95) && ((-getChannel(S_YAW)/350.0) > 0.9))
	{
		dbg_print("motor on\n");
		*motorbool = 1;
	}else if(((getChannel(S_THROTTLE)/350.0)<-0.95) && ((-getChannel(S_YAW)/350.0) <-0.9))
	{
		dbg_print("motor off\n");
		*motorbool = -1;
	}else{
		//dbg_print("motor nothing\n");
		*motorbool = 0;
	}
	
	switch (checkReceivers())
	{
		case 1:
		rc_check = 1;
		break;
		case -1:
		rc_check = -1;
		break;
		case -2:
		rc_check = -2;
		break;
	}
	//dbg_print("rc_check: ");
	//dbg_print_num(rc_check,10);
	//dbg_print("; motorbool : ");
	//dbg_print_num(*motorbool,10);
	//dbg_print("\n");
}

task_return_t set_mav_mode_n_state()
{
	uint8_t channelSwitches = 0;
	int8_t RC_check = 0;
	int8_t motor_switch = 0;
	
	rc_user_channels(&channelSwitches,&RC_check, &motor_switch);
	
	switch(board->mav_state)
	{
		case MAV_STATE_CALIBRATING:
		break;
		case MAV_STATE_STANDBY:
		if (motor_switch == 1)
		{
			switch(channelSwitches)
			{
				case 0:
					//dbg_print("Switching on the motors!");
					board->controls.run_mode = MOTORS_ON;
					board->mav_state = MAV_STATE_ACTIVE;
					board->mav_mode = MAV_MODE_MANUAL_ARMED;
					break;
				case 1:
					//board->controls.run_mode = MOTORS_ON;
					//board->mav_state = MAV_STATE_ACTIVE;
					//board->mav_mode = MAV_MODE_STABILIZE_ARMED;
					break;
				case 2:
					break;
				case 3:
					break;
			}
		}
		break;
		case MAV_STATE_ACTIVE:
		switch(channelSwitches)
		{
			case 0:
				board->mav_mode= MAV_MODE_STABILIZE_ARMED;
				break;
			case 1:
				board->mav_mode= MAV_MODE_STABILIZE_ARMED;
				break;
			case 2:
				board->mav_mode = MAV_MODE_GUIDED_ARMED;
				break;
			case 3:
				board->mav_mode = MAV_MODE_AUTO_ARMED;
				break;
		}
		//dbg_print("motor_switch: ");
		//dbg_print_num(motor_switch,10);
		if (motor_switch == -1)
		{
			//dbg_print("Switch off motors!\n");
			board->controls.run_mode = MOTORS_OFF;
			board->mav_state = MAV_STATE_STANDBY;
			board->mav_mode = MAV_MODE_MANUAL_DISARMED;
		}
		
		switch (RC_check)
		{
			case 1:
				break;
			case -1:
				board->mav_state = MAV_STATE_CRITICAL;
				break;
			case -2:
				board->mav_state = MAV_STATE_EMERGENCY;
				break;
		}
		break;
		case MAV_STATE_CRITICAL:
		switch(channelSwitches)
		{
			case 0:
				board->mav_mode= MAV_MODE_STABILIZE_ARMED;
				break;
			case 1:
				board->mav_mode= MAV_MODE_STABILIZE_ARMED;
				break;
			case 2:
				board->mav_mode = MAV_MODE_GUIDED_ARMED;
				break;
			case 3:
				break;
		}
		if (motor_switch == -1)
		{
			board->controls.run_mode = MOTORS_OFF;
			board->mav_state = MAV_STATE_STANDBY;
			board->mav_mode = MAV_MODE_MANUAL_DISARMED;
		}
		switch (RC_check)
		{
			case 1:
				board->mav_state = MAV_STATE_ACTIVE;
				break;
			case -1:
				break;
			case -2:
				board->mav_state = MAV_STATE_EMERGENCY;
				break;
		}
		break;
		case MAV_STATE_EMERGENCY:
			board->mav_mode = MAV_MODE_MANUAL_DISARMED;
			break;
	}

	//dbg_print("MAV state :");
	//dbg_print_num(board->mav_state,10);
	//dbg_print(", MAV mode :");
	//dbg_print_num(board->mav_mode,10);
	//dbg_print("\n");

	// NICOLAS: changed remote model to aircraft to have 5 channels
	////if (getChannel(4)>0) {
	//if (getChannel(5)>0) {
	//board->controls.control_mode = RATE_COMMAND_MODE;
	//} else {
	//board->controls.control_mode = ATTITUDE_COMMAND_MODE;
	//}
	
}

task_return_t run_stabilisation() {
	int i;
	
	if (board->simulation_mode==1) {
		simu_update(&(board->sim_model), &(board->servos), &(board->imu1));
	} else {
		imu_update(&(board->imu1));
	}
	
	switch(board->mav_mode)
	{
		case MAV_MODE_PREFLIGHT:
		case MAV_MODE_MANUAL_ARMED:
			board->controls.rpy[ROLL]=-getChannel(S_ROLL)/350.0;
			board->controls.rpy[PITCH]=-getChannel(S_PITCH)/350.0;
			board->controls.rpy[YAW]=-getChannel(S_YAW)/350.0;
			//board->controls.thrust = min(getChannel(S_THROTTLE)/350.0,board->controls.thrust);
			board->controls.thrust = getChannel(S_THROTTLE)/350.0;
			for (i=0; i<4; i++) {
				board->servos[i].value=SERVO_SCALE*board->controls.thrust;
			}
			
			break;
		case MAV_MODE_STABILIZE_ARMED:
			board->controls.rpy[ROLL]=-getChannel(S_ROLL)/350.0;
			board->controls.rpy[PITCH]=-getChannel(S_PITCH)/350.0;
			board->controls.rpy[YAW]=-getChannel(S_YAW)/350.0;
			//board->controls.thrust = min(getChannel(S_THROTTLE)/350.0,board->controls.thrust);
			board->controls.thrust = getChannel(S_THROTTLE)/350.0;
			quad_stabilise(&(board->imu1), &(board->controls));
			
			break;
		case MAV_MODE_GUIDED_ARMED:
			board->controls.thrust = min(getChannel(S_THROTTLE)/350.0,board->controls.thrust);
			//board->controls.thrust = getChannel(S_THROTTLE)/350.0;
			quad_stabilise(&(board->imu1), &(board->controls));
			
			break;
		case MAV_MODE_AUTO_ARMED:
			board->controls.thrust = min(getChannel(S_THROTTLE)/350.0,board->controls.thrust);
			//board->controls.thrust = getChannel(S_THROTTLE)/350.0;
			quad_stabilise(&(board->imu1), &(board->controls));
			
			break;
		case MAV_MODE_MANUAL_DISARMED:
		case MAV_MODE_STABILIZE_DISARMED:
		case MAV_MODE_GUIDED_DISARMED:
		case MAV_MODE_AUTO_DISARMED:
			//set_servos(&(servo_failsafe));
			for (i=0; i<NUMBER_OF_SERVO_OUTPUTS; i++) {
				board->servos[i]=servo_failsafe[i];
			}
			break;
		
	}
	
	// !!! -- for safety, this should remain the only place where values are written to the servo outputs! --- !!!
	if (board->simulation_mode!=1) {
		set_servos(&(board->servos));
	}
		

}

//task_return_t run_stabilisation() {
	//board->controls.rpy[ROLL]=-getChannel(S_ROLL)/350.0;
	//board->controls.rpy[PITCH]=-getChannel(S_PITCH)/350.0;
	//board->controls.rpy[YAW]=-getChannel(S_YAW)/350.0;
	//board->controls.thrust=getChannel(S_THROTTLE)/350.0;
//
	//imu_update(&(board->imu1));
	//if (getChannel(4)>0) {
		//board->controls.control_mode=RATE_COMMAND_MODE;
	//} else {
		//board->controls.control_mode=ATTITUDE_COMMAND_MODE;
	//}
	//
	//// switch run_mode
	//if ((board->controls.thrust<-0.95) && (board->controls.rpy[YAW]< -0.9)) {
		//board->controls.run_mode=MOTORS_OFF;
		//LED_On(LED1);
	//}
	//if ((board->controls.thrust<-0.95) && (board->controls.rpy[YAW] >0.9)) {
		//board->controls.run_mode=MOTORS_ON;
		//LED_Off(LED1);
	//}
		//
	//quad_stabilise(&(board->imu1), &(board->controls));
	//
	//if (control_input->run_mode==MOTORS_ON) {
		//// send values to servo outputs
		//set_servos(&(board->servos));
	//} else {	
		//set_servos(&(servo_failsafe));
	//}
	//
//
//}

task_return_t gps_task() {
	uint32_t tnow = get_millis();	
	
	gps_update();
	
 	//dbg_print("time :");
 	//dbg_print_num(tnow,10);
 	//dbg_print_num(board->GPS_data.timeLastMsg,10);
 	//dbg_print(" GPS status : 0x");
 	//dbg_print_num(board->GPS_data.status,16);
 	//dbg_print(" status:");
 	//dbg_print_num(board->GPS_data.accuracyStatus,10);
 	//dbg_print_num(board->GPS_data.horizontalStatus,10);
 	//dbg_print_num(board->GPS_data.altitudeStatus,10);
 	//dbg_print_num(board->GPS_data.speedStatus,10);
 	//dbg_print_num(board->GPS_data.courseStatus,10);
 	//dbg_print("\n");
	//
	if(newValidGpsMsg())
	{/*
		dbg_print("GPS status:");
		dbg_print_num(board->GPS_data.status,10);
		dbg_print(" time gps:");
		dbg_print_num(board->GPS_data.timegps,10);
		dbg_print(" latitude :");
		dbg_print_num(board->GPS_data.latitude,10);
		dbg_print(" longitude :");
		dbg_print_num(board->GPS_data.longitude,10);
		dbg_print(" altitude");
		dbg_print_num(board->GPS_data.altitude,10);
		dbg_print("\n");*/
	}
}

task_return_t run_estimator()
{
	estimator_loop();
	
}

task_return_t run_navigation_task()
{
	int8_t i;
	
	if (((board->mav_state == MAV_STATE_ACTIVE)||(board->mav_state == MAV_STATE_CRITICAL))&&((board->mav_mode == MAV_MODE_AUTO_ARMED)||(board->mav_mode == MAV_MODE_GUIDED_ARMED)))
	{
		run_navigation();
	}
	
	
	//if ((board->number_of_waypoints > 0)&& waypoint_receiving == 0 )
	//{
		//dbg_print("List of Waypoint:");
		//for (i=0; i<board->number_of_waypoints; i++)
		//{
			//dbg_print("wp_id:");
			//dbg_print_num(board->waypoint_list[i].wp_id,10);
			//dbg_print(" autocontinue:");
			//dbg_print_num(board->waypoint_list[i].autocontinue,10);
			//dbg_print(" current:");
			//dbg_print_num(board->waypoint_list[i].current,10);
			//dbg_print(" frame:");
			//dbg_print_num(board->waypoint_list[i].frame,10);
			//dbg_print(" x:");
			//dbg_print_num(board->waypoint_list[i].x,10);
			//dbg_print(" y:");
			//dbg_print_num(board->waypoint_list[i].y,10);
			//dbg_print(" z:");
			//dbg_print_num(board->waypoint_list[i].z,10);
			//dbg_print(" params:");
			//dbg_print_num(board->waypoint_list[i].param1,10);
			//dbg_print_num(board->waypoint_list[i].param2,10);
			//dbg_print_num(board->waypoint_list[i].param3,10);
			//dbg_print_num(board->waypoint_list[i].param4,10);
			//dbg_print(";");
		//}
		//dbg_print("\n");
		//board->number_of_waypoints = 0;
	//}
	
}

task_return_t send_rt_stats() {
	
	mavlink_msg_named_value_float_send(MAVLINK_COMM_0, get_millis(), "stabAvgDelay", main_tasks.tasks[1].delay_avg);
	mavlink_msg_named_value_float_send(MAVLINK_COMM_0, get_millis(), "stabDelayVar", sqrt(main_tasks.tasks[1].delay_var_squared));
	mavlink_msg_named_value_float_send(MAVLINK_COMM_0, get_millis(), "stabMaxDelay", main_tasks.tasks[1].delay_max);
	mavlink_msg_named_value_float_send(MAVLINK_COMM_0, get_millis(), "stabRTvio", main_tasks.tasks[0].rt_violations);

	mavlink_msg_named_value_float_send(MAVLINK_COMM_0, get_millis(), "imuExTime", main_tasks.tasks[0].execution_time);
	mavlink_msg_named_value_float_send(MAVLINK_COMM_0, get_millis(), "stabExTime", main_tasks.tasks[1].execution_time);
	mavlink_msg_named_value_float_send(MAVLINK_COMM_0, get_millis(), "estExTime", main_tasks.tasks[3].execution_time);
	mavlink_msg_named_value_float_send(MAVLINK_COMM_0, get_millis(), "MVLExTime", main_tasks.tasks[9].execution_time);

	
	main_tasks.tasks[1].rt_violations=0;
	main_tasks.tasks[1].delay_max=0;

}

void initialisation() {
	int i;
	irq_initialize_vectors();
	cpu_irq_enable();
	Disable_global_interrupt();
	
	enum GPS_Engine_Setting engine_nav_settings = GPS_ENGINE_AIRBORNE_4G;
	
	// Initialize the sleep manager
	sleepmgr_init();

	sysclk_init();
	board_init();
	delay_init(sysclk_get_cpu_hz());
	init_time_keeper();

	
	INTC_init_interrupts();
	
	
	if (init_i2c(0)!=STATUS_OK) {
		//putstring(STDOUT, "Error initialising I2C\n");
		while (1==1);
	} else {
		//putstring(STDOUT, "initialised I2C.\n");
	};
	if (init_i2c(1)!=STATUS_OK) {
		//putstring(STDOUT, "Error initialising I2C\n");
		while (1==1);
	} else {
		//putstring(STDOUT, "initialised I2C.\n");
	};

	init_radar_modules();

	board=initialise_board();

	//init_gps_ubx(engine_nav_settings);
	
	
	Enable_global_interrupt();
		
	dbg_print("Debug stream initialised\n");

	
	
/*
	set_servo(0, -500, -500);
	set_servo(1, -500, -500);
	set_servo(2, -500, -500);
	set_servo(3, -500, -500);
*/
	
	set_servos(&servo_failsafe);
	
	//delay_ms(1000);
	init_stabilisation();


	init_onboard_parameters();
	init_mavlink_actions();
	
	LED_Off(LED1);
	
	board->imu1.attitude.calibration_level=LEVELING;	
	for (i=400; i>0; i--) {
		imu_update(&board->imu1);
		if (i%50 ==0) {
			// Send heartbeat message
			board->mav_state = MAV_STATE_CALIBRATING;
			board->mav_mode = MAV_MODE_PREFLIGHT;
			mavlink_msg_heartbeat_send(MAVLINK_COMM_0, MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, MAV_MODE_PREFLIGHT, 0, MAV_STATE_CALIBRATING);
		}
		
		delay_ms(5);
	}
	// after initial leveling, initialise accelerometer biases
	board->imu1.attitude.calibration_level=LEVEL_PLUS_ACCEL;
	for (i=200; i>0; i--) {
		imu_update(&board->imu1);
		if (i%50 ==0) {
			// Send heartbeat message
			board->mav_state = MAV_STATE_CALIBRATING;
			board->mav_mode = MAV_MODE_PREFLIGHT;
			mavlink_msg_heartbeat_send(MAVLINK_COMM_0, MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, MAV_MODE_PREFLIGHT, 0, MAV_STATE_CALIBRATING);
		}
			
		delay_ms(5);
	}
	board->imu1.attitude.calibration_level=OFF;
	//reset position estimate
	for (i=0; i<3; i++) {
		// clean acceleration estimate without gravity:
		board->imu1.attitude.vel_bf[i]=0.0;
		board->imu1.attitude.vel[i]=0.0;
		board->imu1.attitude.pos[i]=0.0;
	}
	board->mav_state = MAV_STATE_STANDBY;
	board->mav_mode = MAV_MODE_MANUAL_DISARMED;
	
	e_init();
}

void main (void)
{
	int i=0;
	int counter=0;
	uint32_t last_looptime, this_looptime;
	
	initialisation();
	
	init_scheduler(&main_tasks);
	
//	register_task(&main_tasks, 0, 4000, RUN_REGULAR, &run_imu_update );
	register_task(&main_tasks, 1, 4000, RUN_REGULAR, &run_stabilisation );
	register_task(&main_tasks, 2, 1000, RUN_REGULAR, &mavlink_protocol_update);
	
	//register_task(&main_tasks, 3 ,100000, RUN_REGULAR, &gps_task);
	//register_task(&main_tasks, 4, 10000, RUN_REGULAR, &run_estimator);
	//register_task(&main_tasks, 4, 100000, RUN_REGULAR, &read_radar);

	register_task(&main_tasks, 5, 1000000, RUN_REGULAR, &run_navigation_task);

	register_task(&main_tasks, 6, 1000000, RUN_REGULAR, &set_mav_mode_n_state);

	//add_task(get_mavlink_taskset(),  1000000, RUN_NEVER, &send_rt_stats, MAVLINK_MSG_ID_NAMED_VALUE_FLOAT);
	
	
	// turn on simulation mode
	board->simulation_mode=1;
	// main loop
	counter=0;
	while (1==1) {
		this_looptime=get_millis();
		
		run_scheduler_update(&main_tasks, ROUND_ROBIN);
		
		LED_On(LED1);


		counter=(counter+1)%1000;
		last_looptime=this_looptime;	
	}		
}


