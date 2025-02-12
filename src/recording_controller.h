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

#pragma once

#include <atomic>
#include <thread>
#include <condition_variable>
#include <queue>
#include <chrono>

class recording_controller
{
	public:
		enum class state
		{
			started,
			stopped,
			paused
		};

		recording_controller();
		~recording_controller();

		//No copying
		recording_controller(const recording_controller& other) = delete;
		recording_controller& operator = (const recording_controller& other) = delete;

		recording_controller(recording_controller&& other) = default;
		recording_controller& operator = (recording_controller&& other) = default;
	public:
		void start_recording(std::chrono::milliseconds time = std::chrono::milliseconds{ 0 });
		void stop_recording(std::chrono::milliseconds time = std::chrono::milliseconds{ 0 });

		state get_current_state();

	protected:

	private:
		void work();
		void abort();

		std::thread m_thread;
		std::condition_variable m_begin_task;
		std::condition_variable m_wait_task;

		mutable std::mutex m_task_mutex;
		mutable std::mutex m_wait_mutex;
		mutable std::mutex m_state_mutex;

		std::atomic<state> m_pending_state;
		std::atomic<std::chrono::milliseconds> m_state_change_time;

		std::atomic_bool m_exit;
		std::atomic_bool m_start_requested;
		std::atomic_bool m_cancel;
		std::atomic_bool m_state_change_pending;
};