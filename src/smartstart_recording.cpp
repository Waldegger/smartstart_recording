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

#include "smartstart_recording.h"

#include <QAction>     
#include <QMenu>
#include <QMessageBox>

#include <chrono>
#include <memory>

#include "plugin_window.h"
#include "constants.h"

smartstart_recording::smartstart_recording()
	: m_dirty{ false }
{ }

smartstart_recording& smartstart_recording::get()
{
	static smartstart_recording instance{};

	return instance;
}

bool smartstart_recording::load()
{
	auto* action = static_cast<QAction*>(obs_frontend_add_tools_menu_qaction(obs_module_text(PLUGIN_NAME.data())));

	auto cb = []
		{
			//Code for opening window for plugin here
			obs_frontend_push_ui_translation(obs_module_get_string);

			auto my_window = new plugin_window(static_cast<QMainWindow*>(obs_frontend_get_main_window()));

			my_window->setAttribute(Qt::WA_DeleteOnClose);
			my_window->show();

			obs_frontend_pop_ui_translation();
		};

	QAction::connect(action, &QAction::triggered, cb);

	obs_frontend_add_save_callback(obs_frontend_save_load_handler, nullptr);
	obs_frontend_add_event_callback(obs_frontend_event_handler, nullptr);
	signal_handler_connect(obs_get_signal_handler(), "source_rename", obs_source_rename_handler, nullptr);

	return true;
}

void smartstart_recording::unload()
{
	
}

void smartstart_recording::update_recording_settings(const std::list<recording_setting>& new_list)
{
	m_recording_setting_list = new_list;
	build_recording_table();

	m_dirty = true;
}

void smartstart_recording::remove_recording_setting(std::string_view name)
{
	m_recording_setting_map.erase(name.data());
	m_recording_setting_list.remove_if([&name](const recording_setting& item) -> bool {return item.get_scene_name() == name.data(); });
}

const std::list<recording_setting>& smartstart_recording::get_recording_setting_list() const
{
	return m_recording_setting_list;
}

const recording_setting* smartstart_recording::get_recording_setting(const std::string_view scene_name) const
{
	auto it = m_recording_setting_map.find(scene_name.data());

	if (it != m_recording_setting_map.end())
		return it->second;

	return nullptr;
}

void smartstart_recording::save_load_handler(obs_data_t* save_data, bool saving, void* user_data)
{
	constexpr std::string_view SETTING_NAME = "recording_setting_table";
	constexpr std::string_view SETTING_ARRAY_NAME = "recording_settings";
	constexpr std::string_view SCENE_NAME = "scene_name";
	constexpr std::string_view ACTION = "action";
	constexpr std::string_view TRIGGER_TIME = "trigger_time";

	if (saving)
	{
		if (m_dirty)
		{
			auto obj_ptr = std::unique_ptr<obs_data_t, std::function<void(obs_data_t*)>>(obs_data_create(), [](obs_data_t* ptr) -> void {obs_data_release(ptr); });
			auto array_ptr = std::unique_ptr<obs_data_array_t, std::function<void(obs_data_array_t*)>>(obs_data_array_create(), [](obs_data_array_t* ptr) -> void {obs_data_array_release(ptr); });

			obs_data_set_obj(save_data, SETTING_NAME.data(), obj_ptr.get());

			for (auto& v : m_recording_setting_list)
			{
				auto recording_setting_obj_ptr = std::unique_ptr<obs_data_t, std::function<void(obs_data_t*)>>(obs_data_create(), [](obs_data_t* ptr) -> void {obs_data_release(ptr); });

				obs_data_set_string(recording_setting_obj_ptr.get(), SCENE_NAME.data(), v.get_scene_name().c_str());
				obs_data_set_int(recording_setting_obj_ptr.get(), ACTION.data(), static_cast<std::underlying_type_t<recording_setting::action>>(v.get_action()));
				obs_data_set_int(recording_setting_obj_ptr.get(), TRIGGER_TIME.data(), v.get_trigger_time());
				obs_data_array_push_back(array_ptr.get(), recording_setting_obj_ptr.get());
			}
			obs_data_set_array(obj_ptr.get(), SETTING_ARRAY_NAME.data(), array_ptr.get());
		
			m_dirty = false;
		}
	}
	else
	{
		m_recording_setting_list.clear();
		auto obj_ptr = std::unique_ptr<obs_data_t, std::function<void(obs_data_t*)>>(obs_data_get_obj(save_data, SETTING_NAME.data()), [](obs_data_t* ptr) -> void {obs_data_release(ptr); });
		if (obj_ptr)
		{
			auto settings_array_ptr = std::unique_ptr<obs_data_array_t, std::function<void(obs_data_array_t*)>>(obs_data_get_array(obj_ptr.get(), SETTING_ARRAY_NAME.data()), [](obs_data_array_t* ptr) -> void {obs_data_array_release(ptr); });
			if (settings_array_ptr)
			{
				size_t count = obs_data_array_count(settings_array_ptr.get());

				for (size_t i = 0; i < count; ++i)
				{
					auto recording_setting_obj_ptr = std::unique_ptr<obs_data_t, std::function<void(obs_data_t*)>>(obs_data_array_item(settings_array_ptr.get(), i), [](obs_data_t* ptr) -> void {obs_data_release(ptr); });
					auto setting = recording_setting_obj_ptr.get();

					auto scene_name = obs_data_get_string(setting, SCENE_NAME.data());
					auto action = static_cast<recording_setting::action>(obs_data_get_int(setting, ACTION.data()));
					auto trigger_time = static_cast<uint32_t>(obs_data_get_int(setting, TRIGGER_TIME.data()));

					m_recording_setting_list.emplace_back(recording_setting{ scene_name, action, trigger_time });
				}
			}
		}

		build_recording_table();
		m_dirty = false;
	}
}

void smartstart_recording::event_handler(obs_frontend_event event, void* data)
{
	auto connect_transition_handlers = []() -> void
		{
			//Set the transition handler to each transition
			obs_frontend_source_list transition_list{ 0 };
			obs_frontend_get_transitions(&transition_list);

			for (size_t i = 0; i < transition_list.sources.num; ++i)
			{
				const auto& v = transition_list.sources.array[i];
				signal_handler_connect(obs_source_get_signal_handler(v), "transition_start", obs_source_transistion_start_handler, v);
			}

			obs_frontend_source_list_free(&transition_list);
		};

	auto scene_list_changed_hander = [this]() -> void
		{
			auto scene_list = std::unique_ptr<char*, std::function<void(char**)>>(obs_frontend_get_scene_names(), [](char** ptr)->void { bfree(ptr); });

			for (auto it = m_recording_setting_list.begin(); it != m_recording_setting_list.end(); it++)
			{
				bool item_found = false;
				auto& v = *(it);

				for (size_t i = 0; scene_list.get()[i]; ++i)
				{
					auto item = scene_list.get()[i];
					if (item == v.get_scene_name())
					{
						item_found = true;
						break;
					}
				}

				if (!item_found)
				{
					m_recording_setting_map.erase(v.get_scene_name());
					m_recording_setting_list.erase(it++);
					m_dirty = true;
				}
			}

			if (m_dirty)
			{
				obs_frontend_save();
			}
		};

	switch (event)
	{
		case OBS_FRONTEND_EVENT_SCENE_CHANGED:
		{
			auto ptr = std::unique_ptr<obs_source_t, std::function<void(obs_source_t*)>>(obs_frontend_get_current_scene(), [](obs_source_t* ptr)->void {obs_source_release(ptr); });
			on_scene_changed(ptr.get(), nullptr);
		}
		break;

		case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
		{
			scene_list_changed_hander();
		}
		break;

		case OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED:
		{
			connect_transition_handlers();
		}
		break;

		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED:
		{
			connect_transition_handlers();
		}
		break;

		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CLEANUP:
		case OBS_FRONTEND_EVENT_EXIT:
		{
			//Cleanup is happening here
		}
		break;

		default:
		{

		}
		break;
	}
}

void smartstart_recording::transistion_start_handler(void* data, calldata_t* call_data)
{
	auto transition = static_cast<obs_source_t*>(data);

	auto source = obs_transition_get_source(transition, OBS_TRANSITION_SOURCE_B);
	on_scene_changed(source, transition);
	obs_source_release(source);
}

void smartstart_recording::source_rename_handler(void* data, calldata_t* call_data)
{
	std::string new_name = calldata_string(call_data, "new_name");
	std::string prev_name = calldata_string(call_data, "prev_name");

	auto item = m_recording_setting_map.find(prev_name);
	if (item != m_recording_setting_map.end())
	{
		auto old_item = item->second;
		old_item->set_scene_name(new_name);

		m_recording_setting_map.erase(prev_name);
		m_recording_setting_map[new_name] = old_item;

		m_dirty = true;
	}
}

void smartstart_recording::on_scene_changed(const obs_source_t* source, const obs_source_t* transition)
{
	if (!source)
		return;

	std::string_view source_name = obs_source_get_name(source);
	
	//scene change can happen directly after the transition. Lets remember which scene was handeled last
	if (m_last_handeled_scene_name == source_name)
		return;

	m_last_handeled_scene_name = source_name;

	//QMessageBox::information(static_cast<QMainWindow*>(obs_frontend_get_main_window()), "Scene Name", source_name.data());

	auto rec_setting = get_recording_setting(source_name);
	//Without any setting, there is nothgin to do
	if (!rec_setting)
		return;
	
	if (transition)
	{
		switch (rec_setting->get_action())
		{
			case recording_setting::action::start:
			{
				m_recording_controller.start_recording(std::chrono::milliseconds{ rec_setting->get_trigger_time() });
			}
			break;

			default:
			{
				m_recording_controller.stop_recording(std::chrono::milliseconds{ rec_setting->get_trigger_time() });
			}
			break;
		}
		
		return;
	}

	//Without transition we want to immediatley start the recording if requested (probably we are here, because OBS crashed)
	if(rec_setting->get_action() == recording_setting::action::start)
		m_recording_controller.start_recording(std::chrono::milliseconds{ 0 });
}

void smartstart_recording::build_recording_table()
{
	m_recording_setting_map.clear();

	for (auto& v : m_recording_setting_list)
		m_recording_setting_map[v.get_scene_name()] = &v;
}

void smartstart_recording::obs_frontend_save_load_handler(obs_data_t* save_data, bool saving, void* user_data)
{
	get().save_load_handler(save_data, saving, user_data);
}

void smartstart_recording::obs_frontend_event_handler(obs_frontend_event event, void* user_data)
{
	get().event_handler(event, user_data);
}

void smartstart_recording::obs_source_transistion_start_handler(void* data, calldata_t* call_data)
{
	get().transistion_start_handler(data, call_data);
}

void smartstart_recording::obs_source_rename_handler(void* data, calldata_t* call_data)
{
	get().source_rename_handler(data, call_data);
}
