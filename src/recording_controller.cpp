/*
SmartStart Recording
Copyright (C) <2025> <ShivaPlays> <stefan.waldegger@yahoo.de>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include "recording_controller.h"

#include <obs-frontend-api.h>
#include <obs-module.h> 

recording_controller::recording_controller()
	: m_thread{ &recording_controller::work, this }
	, m_pending_state{ state::stopped }
	, m_state_change_time{ std::chrono::milliseconds{0} }
	, m_exit{ false }
	, m_start_requested{ false }
	, m_cancel{ false }
	, m_state_change_pending{ false }
{ }

recording_controller::~recording_controller()
{
	m_exit = true;
	m_begin_task.notify_all();
	m_wait_task.notify_all();
}

void recording_controller::start_recording(std::chrono::milliseconds time)
{
	//abort if the new state is going to be stopped
	abort();

	if (time == std::chrono::milliseconds{ 0 })
	{
		std::unique_lock lock{ m_state_mutex };
		obs_frontend_recording_start();

		return;
	}

	if (get_current_state() != state::started)
	{
		m_start_requested = true;
		m_state_change_time = time;
		m_pending_state = state::started;
		m_begin_task.notify_all();
	}
}

void recording_controller::stop_recording(std::chrono::milliseconds time)
{
	//abort if the new state is going to be started
	abort();

	//We dont need our thread to work if the state change is wanted immediatley
	if (time == std::chrono::milliseconds{ 0 })
	{
		std::unique_lock lock{ m_state_mutex };
		obs_frontend_recording_stop();

		return;
	}

	if (get_current_state() != state::stopped)
	{
		m_start_requested = true;
		m_state_change_time = time;
		m_pending_state = state::stopped;
		m_begin_task.notify_all();
	}
}

void recording_controller::abort()
{
	if (!m_state_change_pending)
		return;

	m_cancel = true;
	m_wait_task.notify_one();
}

recording_controller::state recording_controller::get_current_state()
{
	std::unique_lock lock{ m_state_mutex };
	
	if (obs_frontend_recording_active())
		return state::started;

	if (obs_frontend_recording_paused())
		return state::paused;

	return state::stopped;
}

void recording_controller::work()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock{ m_task_mutex };
		m_begin_task.wait(lock, [this]() -> bool {return m_start_requested.exchange(false) || m_exit; });

		if (m_exit)
			break;

		std::unique_lock<std::mutex> wait_lock{ m_wait_mutex };
		m_state_change_pending = true;
		m_wait_task.wait_for(wait_lock, m_state_change_time.load(), [this]() -> bool {return m_cancel || m_exit; });

		if (m_exit)
			break;

		if (m_cancel.exchange(false))
		{
			m_state_change_pending = false;
			continue;
		}

		{
			std::unique_lock lock{ m_state_mutex };
			if (m_pending_state == state::started)
				obs_frontend_recording_start();
			else
				obs_frontend_recording_stop();
		}
		
		m_state_change_pending = false;
	}
}