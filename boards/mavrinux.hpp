/*******************************************************************************
 * Copyright (c) 2009-2014, MAV'RIC Development Team
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

/*******************************************************************************
 * \file 	mavrinux.hpp
 * 
 * \author 	MAV'RIC Team
 *   
 * \brief 	Emulated board running on linux
 *
 ******************************************************************************/


#ifndef MAVRINUX_HPP_
#define MAVRINUX_HPP_


#include "imu.hpp"

#include "gpio_dummy.hpp"
#include "serial_dummy.hpp"
#include "spektrum_satellite.hpp"

#include "simulation.hpp"
#include "dynamic_model_quad_diag.hpp"

#include "serial_linux_io.hpp"
#include "serial_udp.hpp"
#include "file_linux.hpp"

#include "adc_dummy.hpp"
#include "battery.hpp"

extern "C"
{
	#include "streams.h"
	#include "servos.h"
}


/**
 * \brief 	Configuration structure
 */
typedef struct
{
	imu_conf_t				imu_config;
} mavrinux_conf_t;


/**
 * \brief 	Default configuration for the board
 * 
 * \return 	Config structure
 */
static inline mavrinux_conf_t mavrinux_default_config();


/**
 * \brief  Emulated board running on linux
 * 
 */
class Mavrinux
{
public:
	/**
	 * \brief  			Constructor
	 * 
	 * \param 	config 	Board configuration
	 */
	Mavrinux( mavrinux_conf_t config = mavrinux_default_config() );


	/**
	 * \brief  	Hardware initialisation 
	 *
	 * \return 	Success 
	 */
	bool init(void);
	

	/**
	 * Public Members
	 */
	servos_t					servos;
	Dynamic_model_quad_diag		dynamic_model;
	Simulation					sim;
	Imu 						imu;

	Adc_dummy 			adc_battery;
	Battery 			battery;

	Gpio_dummy			dsm_receiver_pin;
	Gpio_dummy			dsm_power_pin;
	Serial_dummy 		dsm_serial;
	Spektrum_satellite	spektrum_satellite;

	Serial_udp			mavlink_serial;		
	Serial_linux_io 	debug_serial;

	File_linux 			file_flash;
	File_linux 			file_log;

private:
	byte_stream_t	dbg_stream_;  ///< Temporary member to make print_util work TODO: remove
};


/**
 * \brief 	Default configuration for the board
 * 
 * \return 	Config structure
 */
static inline mavrinux_conf_t mavrinux_default_config()
{
	mavrinux_conf_t conf = {};

	// -------------------------------------------------------------------------
	// Imu config
	// -------------------------------------------------------------------------
	conf.imu_config	= imu_default_config();

	return conf;
}


#endif /* MAVRINUX_HPP_ */
