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

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>

#include <optional>

#include "recording_setting.h"

class record_edit_window : public QDialog
{
public:
	record_edit_window(QWidget* parent = nullptr, Qt::WindowFlags flags = { 0 });
public:
	inline void set_recording_setting(const recording_setting& value) { m_recording_setting = value; }
	inline const std::optional<recording_setting>& get_recording_setting() const { return m_recording_setting; }

protected:
	virtual void showEvent(QShowEvent* ev) override;
private:
	QComboBox m_scene_names_combo_box{ this };
	QComboBox m_record_action_combobox{ this };
	QSpinBox m_timing_spin_box{ this };

	std::optional<recording_setting> m_recording_setting;
};