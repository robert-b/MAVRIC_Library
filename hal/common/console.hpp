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
 * \file  	console.hpp
 * 
 * \author  MAV'RIC Team
 *   
 * \brief   Write to any write-able module in human-readable format
 *
 ******************************************************************************/

#ifndef CONSOLE_HPP_
#define CONSOLE_HPP_

#include <stdint.h>


/**
 * \brief 	Class template to write to any write-able module in human-readable format
 */
template <typename Writeable>
class Console 
{
private:
	Writeable& stream_;

public:
	/**
	 * \brief Constructor
	 */
	Console(Writeable& stream);


	/**
	 * \brief 	Write to the console
	 *
	 * \param 	data 	The buffer to write.
	 * \param 	size 	The number of bytes to write.
	 * 
	 * \return 	success
	 */
	bool write(const uint8_t* data, uint32_t size);

	/**
	 * \brief 	Write to the console
	 *
	 * \param 	text 	Text to write to console
	 * 
	 * \return 	success
	 */
	bool write(const char* text);


	/**
	 * \brief 	Write bool to the console ("true"/"false")
	 *
	 * \param 	value 	boolean value to be evaluated
	 * 
	 * \return 	success
	 */
	bool write(bool value);

	/**
	 * \brief 	Write floating point to the console (wrapper for write_floating(..))
	 *
	 * \param 	number 	
	 * 
	 * \return 	success
	 */
	bool write(float number, uint8_t after_digits = 3);
	bool write(double number, uint8_t after_digits = 3);

	/**
	 * \brief 	Write integer number to the console (wrapper for write_integer(..))
	 *
	 * \param 	number 	
	 * 
	 * \return 	success
	 */
	bool write(uint32_t);
	bool write(int32_t);
	bool write(uint16_t);
	bool write(int16_t);
	bool write(uint8_t);
	bool write(int8_t);
	bool write(int);

	/**
	 * \brief 	Write floating point to the console
	 *
	 * \param 	number 	floating point number (float/double)
	 * \param 	after_digits 	digits after decimal point
	 * 
	 * \return 	success
	 */
	template <typename T>
	bool write_floating(T number, uint8_t after_digits = 3);

	/**
	 * \brief 	Write integer number to the console
	 *
	 * \param 	number 	integer number (uintX_t/intX_t)
	 * 
	 * \return 	success
	 */
	template<typename T>
	bool write_integer(T number);



	/**
	 * \brief 	Flushes the buffer of the console
	 * 
	 * \return 	success
	 */
	void flush();


	

	template<typename T>
	Console<Writeable> &operator<<(const T &a);	

	/* definition of ConsoleManipulator function pointer (used for "console << endl") */
	typedef Console<Writeable>& (*ConsoleManipulator)(Console<Writeable>&);

	Console<Writeable> &operator<<(ConsoleManipulator manip);
};

/* definition of 'endl' */
template<typename Writeable>
Console<Writeable>& endl(Console<Writeable>& console);

// Template implementation file
#include "console.hxx"

#endif /* CONSOLE_HPP_ */