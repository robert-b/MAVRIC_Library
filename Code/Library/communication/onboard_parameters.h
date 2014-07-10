/**
 * \page The MAV'RIC License
 *
 * The MAV'RIC Framework
 *
 * Copyright © 2011-2014
 *
 * Laboratory of Intelligent Systems, EPFL
 */


/**
 * \file onboard_parameters.h
 * 
 * Mav'ric Onboard parameters
 */


#ifndef ONBOARD_PARAMETERS_H_
#define ONBOARD_PARAMETERS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mavlink_stream.h"
#include "scheduler.h"

#include <stdbool.h>

#define MAX_ONBOARD_PARAM_COUNT 120	// should be < 122 to fit on user page on AT32UC3C1512

#define MAVERIC_FLASHC_USER_PAGE_START_ADDRESS AVR32_FLASHC_USER_PAGE_ADDRESS + 0x04	// +4bytes for unknown reason
#define MAVERIC_FLASHC_USER_PAGE_FREE_SPACE 500	// 	512bytes user page, 
												//	-4bytes at the start, 
												//  -8bytes for the protected fuses at the end of the user page
												
/**
 * \brief	Structure of onboard parameter.
 */
typedef struct
{
	float* param;												///< Pointer to the parameter value
	char param_name[MAVLINK_MSG_PARAM_SET_FIELD_PARAM_ID_LEN];	///< Parameter name composed of 16 characters
	mavlink_message_type_t data_type;							///< Parameter type
	uint8_t param_name_length;									///< Length of the parameter name
	uint8_t param_id;											///< Parameter ID
	bool  schedule_for_transmission;							///< Boolean to activate the transmission of the parameter
} onboard_parameter_t;


/**
 * \brief	Set of parameter composed of onboard parameters and number of parameters.
 */
typedef struct 
{
	onboard_parameter_t parameters[MAX_ONBOARD_PARAM_COUNT];	///< Onboard parameters array
	uint16_t param_count;										///< Number of onboard parameter effectively in the array
} onboard_parameter_set_t;											


/**
 * \brief	TODO: Modify the name of this structure to make it sized as the free flash memory to store these parameters
 */															
typedef struct												
{
	float values[MAVERIC_FLASHC_USER_PAGE_FREE_SPACE];
} nvram_data_t;


// TODO: update documentation to new function prototypes

/**
* \brief	Initialisation of the Parameter_Set structure by setting the number of onboard parameter to 0
*/
void onboard_parameters_init(onboard_parameter_set_t* param_set);

/**
 * \brief				Register parameter in the internal parameter list that gets published to MAVlink
 *
 * \param val			Unsigned 8 - bits integer parameter value
 * \param param_name	Name of the parameter
 */
void onboard_parameters_add_parameter_uint8(onboard_parameter_set_t* param_set, uint8_t* val, const char* param_name);

/**
 * \brief				Register parameter in the internal parameter list that gets published to MAVlink
 *
 * \param val			Unsigned 32 - bits integer parameter value
 * \param param_name	Name of the parameter
 */
void onboard_parameters_add_parameter_uint32(onboard_parameter_set_t* param_set, uint32_t* val, const char* param_name);

/**
 * \brief				Register parameter in the internal parameter list that gets published to MAVlink
 *
 * \param val			Signed 32 - bits integer parameter value
 * \param param_name	Name of the parameter
 */
void onboard_parameters_add_parameter_int32(onboard_parameter_set_t* param_set, int32_t* val, const char* param_name);

/**
 * \brief				Registers parameter in the internal parameter list that gets published to MAVlink
 *
 * \param val			Floating point parameter value
 * \param param_name	Name of the parameter
 */
void onboard_parameters_add_parameter_float(onboard_parameter_set_t* param_set, float* val, const char* param_name);


/**
 * \brief	Immediately sends all parameters via MAVlink. This might block for a while.
 */
void onboard_parameters_send_all_parameters_now(onboard_parameter_set_t* param_set);

/**
 * \brief	Marks all parameters to be scheduled for transmission
 */
void onboard_parameters_send_all_parameters(onboard_parameter_set_t* param_set);

/**
 * \brief	Sends all parameters that have been scheduled via MAVlink
 */
task_return_t onboard_parameters_send_scheduled_parameters(onboard_parameter_set_t* param_set);

/**
 * \brief			Responds to a MAVlink parameter request
 *
 * \param request	Pointer to the request structure received by MAVlink
 */
void onboard_parameters_send_parameter(onboard_parameter_set_t* param_set, mavlink_param_request_read_t* request);

/**
 * \brief		Responds to a MAVlink parameter set
 *
 * \param rec	Pointer to the received parameter structure
 */
void onboard_parameters_receive_parameter(onboard_parameter_set_t* param_set, mavlink_received_t* rec);

/**
 * \brief		Read onboard parameters from the user page in the flash memory to the RAM memory
 */
void onboard_parameters_read_parameters_from_flashc(onboard_parameter_set_t* param_set);

/**
 * \brief		Write onboard parameters to the RAM memory from the user page in the flash memory
 */
void onboard_parameters_write_parameters_from_flashc(onboard_parameter_set_t* param_set);

#ifdef __cplusplus
}
#endif

#endif /* ONBOARD_PARAMETERS_H */