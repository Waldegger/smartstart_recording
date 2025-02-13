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

#include "record_edit_window.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>

#include <obs-module.h>
#include <obs-frontend-api.h>

#include <sstream>

record_edit_window::record_edit_window(const std::list<recording_setting>& match_list, QWidget* parent, Qt::WindowFlags flags)
	: QDialog(parent, flags)
	, m_match_list{ &match_list }
{
	setWindowTitle(obs_module_text("add_recording_setting"));

	auto grid_layout = new QGridLayout(this);
	grid_layout->setColumnMinimumWidth(0, 300);

	auto scene_select_layout = new QHBoxLayout(this);
	auto action_select_layout = new QHBoxLayout(this);
	auto timing_select_layout = new QHBoxLayout(this);
	auto spacer_layout = new QHBoxLayout(this);
	auto button_layout = new QHBoxLayout(this);

	auto dialog_button_box = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, this);

	m_scene_names_combo_box.setMinimumWidth(300);

	m_record_action_combobox.addItem(obs_module_text("start"), static_cast<std::underlying_type_t<recording_setting::action>>(recording_setting::action::start));
	m_record_action_combobox.addItem(obs_module_text("stop"), static_cast<std::underlying_type_t<recording_setting::action>>(recording_setting::action::stop));
	m_record_action_combobox.setMinimumWidth(300);

	m_timing_spin_box.setMinimum(0);
	m_timing_spin_box.setMaximum(1000000);

	scene_select_layout->addWidget(new QLabel(obs_module_text("recording_edit_window.scene_label"), this));
	scene_select_layout->addWidget(&m_scene_names_combo_box);
	grid_layout->addLayout(scene_select_layout, 0, 0);

	action_select_layout->addWidget(new QLabel(obs_module_text("recording_edit_window.action_label"), this));
	action_select_layout->addWidget(&m_record_action_combobox);
	grid_layout->addLayout(action_select_layout, 1, 0);

	timing_select_layout->addWidget(new QLabel(obs_module_text("recording_edit_window.timing_label"), this));
	timing_select_layout->addWidget(&m_timing_spin_box);
	grid_layout->addLayout(timing_select_layout, 2, 0);
	
	auto spacer_line = new QFrame(this);
	spacer_line->setFrameShape(QFrame::HLine);
	spacer_line->setFrameShadow(QFrame::Sunken);
	spacer_layout->addWidget(spacer_line);
	grid_layout->addLayout(spacer_layout, 3, 0);

	button_layout->addWidget(dialog_button_box);
	grid_layout->addLayout(button_layout, 4, 0);

	auto save_button_click = [this]() -> void
		{
			auto& rec = m_recording_setting.value();

			auto scene_name = m_scene_names_combo_box.itemText(m_scene_names_combo_box.currentIndex());
			auto recording_action = static_cast<recording_setting::action>(m_record_action_combobox.currentData().toInt());
			auto timing = m_timing_spin_box.value();

			rec.set_scene_name(scene_name.toStdString());
			rec.set_action(recording_action);
			rec.set_trigger_time(timing);

			accept();
		};

	auto close_button_click = [this]() -> void
		{
			reject();
		};

	connect(dialog_button_box->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::pressed, save_button_click);
	connect(dialog_button_box->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::pressed, close_button_click);
	
	setLayout(grid_layout);
	layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void record_edit_window::showEvent(QShowEvent* ev)
{
	QDialog::showEvent(ev);

	auto scene_list = std::unique_ptr<char*, std::function<void(char**)>>(obs_frontend_get_scene_names(), [](char** ptr)->void { bfree(ptr); });
	for (size_t i = 0; scene_list.get()[i]; ++i)
	{
		auto item = scene_list.get()[i];

		bool item_found = false;
		for (auto& v : *m_match_list)
		{
			if (item == v.get_scene_name())
			{
				item_found = true;
				continue;
			}
		}

		if (!item_found)
			m_scene_names_combo_box.addItem(item);
	}

	if (!m_recording_setting)
	{
		m_recording_setting = recording_setting{};
		return;
	}

	const auto& rec = m_recording_setting.value();
	m_scene_names_combo_box.addItem(rec.get_scene_name().c_str());
	

	setWindowTitle(obs_module_text("edit_recording_setting"));

	m_scene_names_combo_box.setCurrentText(rec.get_scene_name().c_str());
	m_record_action_combobox.setCurrentIndex(m_record_action_combobox.findData(static_cast<std::underlying_type_t<recording_setting::action>>(rec.get_action())));
	m_timing_spin_box.setValue(static_cast<int>(rec.get_trigger_time()));
}