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
 * \file mission_planner_handler_stop_there.cpp
 *
 * \author MAV'RIC Team
 * \author Matthew Douglas
 *
 * \brief The MAVLink mission planner handler for the stop there state
 *
 ******************************************************************************/


#include "communication/mission_planner_handler_stop_there.hpp"

extern "C"
{

}


//------------------------------------------------------------------------------
// PROTECTED/PRIVATE FUNCTIONS IMPLEMENTATION
//------------------------------------------------------------------------------

void Mission_planner_handler_stop_there::stopping_handler()
{
    float dist2wp_sqr;
    float rel_pos[3];

    rel_pos[X] = (float)(waypoint_hold_coordinates.waypoint.pos[X] - position_estimation_.local_position.pos[X]);
    rel_pos[Y] = (float)(waypoint_hold_coordinates.waypoint.pos[Y] - position_estimation_.local_position.pos[Y]);
    rel_pos[Z] = (float)(waypoint_hold_coordinates.waypoint.pos[Z] - position_estimation_.local_position.pos[Z]);

    dist2wp_sqr = vectors_norm_sqr(rel_pos);
    if (dist2wp_sqr < 25.0f)
    {
        navigation_.internal_state_ = Navigation::NAV_STOP_ON_POSITION;
    }
}

//------------------------------------------------------------------------------
// PUBLIC FUNCTIONS IMPLEMENTATION
//------------------------------------------------------------------------------

Mission_planner_handler_stop_there::Mission_planner_handler_stop_there( Position_estimation& position_estimation_,
                                                                        Navigation& navigation_,
                                                                        State& state_):
            position_estimation_(position_estimation_),
            state_(state_),
            navigation_(navigation_)
{

}

Mission_planner_handler_stop_on_position::handle()
{
    mav_mode_t mode_local = state_.mav_mode();

    stopping_handler();

    if (navigation_.navigation_strategy == Navigation::strategy_t::DUBIN)
    {
        dubin_state_machine(&waypoint_hold_coordinates);
    }

    navigation_.goal = waypoint_hold_coordinates;

    if ((!mav_modes_is_auto(mode_local)) && (!mav_modes_is_guided(mode_local)))
    {
        navigation_.internal_state_ = Navigation::NAV_MANUAL_CTRL;
    }
}