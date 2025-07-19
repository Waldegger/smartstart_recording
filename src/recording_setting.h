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

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>

class recording_setting
{
	public:
		enum class action
		{
			start,
			stop
		};

		recording_setting()
		{ }

		recording_setting(std::string name, action action, uint32_t trigger_time)
			: m_scene_name(name)
			, m_action(action)
			, m_trigger_time(trigger_time)
		{ } 

		friend bool operator==(const recording_setting& lhs, const recording_setting& rhs);
		friend bool operator!=(const recording_setting& lhs, const recording_setting& rhs);

	public:
		inline void set_scene_name(const std::string& name) { m_scene_name = name; } 
		inline const std::string& get_scene_name() const { return m_scene_name; }

		inline void set_action(action action) { m_action = action; }
		inline action get_action() const { return m_action; }

		inline void set_trigger_time(uint32_t trigger_time) { m_trigger_time = trigger_time; }
		inline uint32_t get_trigger_time() const { return m_trigger_time; }

	protected:

	private:
		std::string m_scene_name;
		action m_action = action::start;
		uint32_t m_trigger_time = 0;
};

inline bool operator==(const recording_setting& lhs, const recording_setting& rhs)
{
	return lhs.get_scene_name() == rhs.get_scene_name() 
		&& lhs.get_action() == rhs.get_action() 
		&& lhs.get_trigger_time() == rhs.get_trigger_time();
}

inline bool operator!=(const recording_setting& lhs, const recording_setting& rhs)
{
	return lhs.get_scene_name() != rhs.get_scene_name()
		|| lhs.get_action() != rhs.get_action()
		|| lhs.get_trigger_time() != rhs.get_trigger_time();
}