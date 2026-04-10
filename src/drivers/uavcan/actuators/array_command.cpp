/****************************************************************************
 *
 *   Copyright (C) 2021 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include "array_command.hpp"
#include <systemlib/err.h>
#include <drivers/drv_hrt.h>

using namespace time_literals;

UavcanArrayCommandController::UavcanArrayCommandController(uavcan::INode &node) :
	_node(node),
	_uavcan_pub_array_cmd(node)
{
	_uavcan_pub_array_cmd.setPriority(UAVCAN_COMMAND_TRANSFER_PRIORITY);
}

void
UavcanArrayCommandController::init()
{
	//Find out what the functions set in the Actuator Commands are, and who they should map to
	char param_name[17];

	//Grab the IDs
	for (unsigned i = 0; i < MAX_ACTUATORS; ++i) {
		snprintf(param_name, sizeof(param_name), "UAVCAN_CMD_ID%d", i);
		_param_handles[i] = param_find(param_name);
	}

	//Grab the functions
	for (unsigned i = 0; i < MAX_ACTUATORS; ++i) {
		snprintf(param_name, sizeof(param_name), "UAVCAN_TYPE%d", i);
		_array_function_handles[i] = param_find(param_name);
	}
}

// void update_outputs(bool stop_motors, uint16_t outputs[MAX_ACTUATORS], unsigned num_outputs);

void
UavcanArrayCommandController::update_outputs(bool stop_motors, uint16_t outputs[MAX_ACTUATORS], unsigned num_outputs)
{
	uavcan::equipment::actuator::ArrayCommand msg1;
	uavcan::equipment::actuator::ArrayCommand msg2;

	if(num_outputs == 16) {
		uint16_t first_array_command = 14;
		uint16_t second_array_command = 16;

		for(uint16_t i = 0; i < first_array_command; i++) {
			uavcan::equipment::actuator::Command cmd;

			//Find which ID this index goes with
			int32_t output_id;
			param_get(_param_handles[i], &output_id);

			// modify here

			// my idea would be to make an array of ArrayCommands and then we loop through the full array of stuff, then we broadcast it out

			if(output_id < 255){
				cmd.actuator_id = output_id;

				//Figure out what type of Command this should be
				int32_t actuator_command_config;
				param_get(_array_function_handles[i], &actuator_command_config);
				cmd.command_type = actuator_command_config;

				cmd.command_value = (float)outputs[i] / 1000.0f; // [0, 1]

				msg1.commands.push_back(cmd);
			}

		}

		for(uint16_t i = 14; i < second_array_command; i++) {
			uavcan::equipment::actuator::Command cmd;

			//Find which ID this index goes with
			int32_t output_id;
			param_get(_param_handles[i], &output_id);

			if(output_id < 255){
				cmd.actuator_id = output_id;

				//Figure out what type of Command this should be
				int32_t actuator_command_config;
				param_get(_array_function_handles[i], &actuator_command_config);
				cmd.command_type = actuator_command_config;

				cmd.command_value = (float)outputs[i] / 1000.0f; // [0, 1]

				msg2.commands.push_back(cmd);
			}
		}
	}

	// for (unsigned i = 0; i < num_outputs; ++i) {
	// 	uavcan::equipment::actuator::Command cmd;

	// 	//Find which ID this index goes with
	// 	int32_t output_id;
	// 	param_get(_param_handles[i], &output_id);

	// 	// modify here

	// 	// my idea would be to make an array of ArrayCommands and then we loop through the full array of stuff, then we broadcast it out

	// 	if(output_id < 255){
	// 		cmd.actuator_id = output_id;

	// 		//Figure out what type of Command this should be
	// 		int32_t actuator_command_config;
	// 		param_get(_array_function_handles[i], &actuator_command_config);
	// 		cmd.command_type = actuator_command_config;

	// 		cmd.command_value = (float)outputs[i] / 1000.0f; // [0, 1]

	// 		msg.commands.push_back(cmd);
	// 	}

	// }

	_uavcan_pub_array_cmd.broadcast(msg1);
	_uavcan_pub_array_cmd.broadcast(msg2);
}
