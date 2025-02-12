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

#include <obs-frontend-api.h>
#include <obs-module.h>

#include <string>
#include <list>
#include <unordered_map>
#include <condition_variable>
#include <mutex>

#include "recording_setting.h"
#include "recording_controller.h"

class smartstart_recording
{
public:
	static smartstart_recording& get();
public:
	bool load();
	void unload();

	void update_recording_settings(const std::list<recording_setting>& new_list);

	const std::list<recording_setting>& get_recording_setting_list() const;
	const recording_setting* get_recording_setting(const std::string_view scene_name) const;

protected:
	smartstart_recording();
private:
	void save_load_handler(obs_data_t* save_data, bool saving, void* user_data);
	void event_handler(obs_frontend_event event, void* data);
	void transistion_start_handler(void* data, calldata_t* call_data);
	void source_rename_handler(void* data, calldata_t* call_data);

	void on_scene_changed(const obs_source_t* source, const obs_source_t* transition);

	void build_recording_table();

	static void obs_frontend_save_load_handler(obs_data_t* save_data, bool saving, void* user_data);
	static void obs_frontend_event_handler(obs_frontend_event event, void* user_data);
	static void obs_source_transistion_start_handler(void* data, calldata_t* call_data);
	static void obs_source_rename_handler(void* data, calldata_t* call_data);

	recording_controller m_recording_controller;

	std::list<recording_setting> m_recording_setting_list;
	std::unordered_map<std::string, recording_setting*> m_recording_setting_map;
	std::string m_last_handeled_scene_name;

	bool m_dirty;
};