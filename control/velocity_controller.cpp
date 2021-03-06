/*******************************************************************************
 * Copyright (c) 2009-2016, MAV'RIC Development Team
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
 * \file velocity_controller_copter.cpp
 *
 * \author MAV'RIC Team
 * \author Felix Schill
 * \author Nicolas Dousse
 * \author Julien Lecoeur
 *
 * \brief A velocity controller for copters.
 *
 ******************************************************************************/

#include <array>

#include "control/velocity_controller.hpp"
#include "util/coord_conventions.hpp"
#include "util/constants.hpp"

extern "C"
{
#include "util/quaternions.h"
#include "util/maths.h"
#include "util/vectors.h"
}


Velocity_controller::Velocity_controller(const args_t& args, const conf_t& config):
    ahrs_(args.ahrs),
    ins_(args.ins),
    velocity_command_(args.velocity_command),
    attitude_command_(args.attitude_command),
    thrust_command_(args.thrust_command),
    control_frame_(config.control_frame)
{
    // Init PID gains
    pid_controller_init(&pid_[X], &config.pid_config[X]);
    pid_controller_init(&pid_[Y], &config.pid_config[Y]);
    pid_controller_init(&pid_[Z], &config.pid_config[Z]);

    // set initial velocity command
    velocity_command_.xyz[X]  = 0.0f;
    velocity_command_.xyz[Y]  = 0.0f;
    velocity_command_.xyz[Z]  = 0.0f;
    velocity_command_.heading = 0.0f;

    // set initial attitude command
    attitude_command_.s     = 1.0f;
    attitude_command_.v[X]  = 0.0f;
    attitude_command_.v[Y]  = 0.0f;
    attitude_command_.v[Z]  = 0.0f;

    // set initial thrust command
    thrust_command_.xyz[X]  = 0.0f;
    thrust_command_.xyz[Y]  = 0.0f;
    thrust_command_.xyz[Z]  = 0.0f;
}


bool Velocity_controller::update(void)
{
    // Get current velocity in NED
    std::array<float,3> velocity = ins_.velocity_lf();

    // Get rotation betwen NED and control frame
    quat_t q_ctrl_frame;
    switch (control_frame_)
    {
        case LOCAL_FRAME:
            q_ctrl_frame = coord_conventions_quaternion_from_rpy(0.0f, 0.0f, 0.0f);
        break;

        case SEMILOCAL_FRAME:
            q_ctrl_frame = coord_conventions_quaternion_from_rpy(0.0f, 0.0f, ahrs_.yaw());
        break;

        case BODY_FRAME:
            q_ctrl_frame = ahrs_.attitude();
        break;
    }

    // Convert command and measurement to control frame
    std::array<float,3> velocity_ctrl_frame;
    std::array<float,3> velocity_command_ctrl_frame;
    quaternions_rotate_vector(quaternions_inverse(q_ctrl_frame), velocity.data(), velocity_ctrl_frame.data());
    quaternions_rotate_vector(quaternions_inverse(q_ctrl_frame), velocity_command_.xyz.data(), velocity_command_ctrl_frame.data());

    // Compute errors in control frame
    float errors[3];
    errors[X] = velocity_command_ctrl_frame[X] - velocity_ctrl_frame[X];
    errors[Y] = velocity_command_ctrl_frame[Y] - velocity_ctrl_frame[Y];
    errors[Z] = velocity_command_ctrl_frame[Z] - velocity_ctrl_frame[Z];

    // Update PID
    std::array<float,3> accel_vector_ctrl_frame;
    accel_vector_ctrl_frame[X] = pid_controller_update(&pid_[X], errors[X]);
    accel_vector_ctrl_frame[Y] = pid_controller_update(&pid_[Y], errors[Y]);
    accel_vector_ctrl_frame[Z] = pid_controller_update(&pid_[Z], errors[Z]);

    // Convert output to NED frame
    std::array<float,3> accel_vector;
    quaternions_rotate_vector(q_ctrl_frame, accel_vector_ctrl_frame.data(), accel_vector.data());

    // Convert accel vector to attitude and thrust command
    bool ret = compute_attitude_and_thrust_from_desired_accel(accel_vector, attitude_command_, thrust_command_);

    return ret;
}


bool Velocity_controller::set_command(const velocity_command_t& vel)
{
    velocity_command_ = vel;
    return true;
}


bool Velocity_controller::get_command(velocity_command_t& vel) const
{
    vel = velocity_command_;
    return true;
}


bool Velocity_controller::get_output(attitude_command_t& att) const
{
    att = attitude_command_;
    return true;
}


bool Velocity_controller::get_output(thrust_command_t& thrust) const
{
    thrust = thrust_command_;
    return true;
}


pid_controller_t& Velocity_controller::get_pid_X(void)
{
    return pid_[X];
}


pid_controller_t& Velocity_controller::get_pid_Y(void)
{
    return pid_[Y];
}


pid_controller_t& Velocity_controller::get_pid_Z(void)
{
    return pid_[Z];
}
