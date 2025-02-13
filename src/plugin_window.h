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

#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>

#include <list>

#include "recording_setting.h"

class plugin_window : public QMainWindow
{
	public:
		plugin_window(QWidget* parent = nullptr, Qt::WindowFlags flags = {0});
	public:

	protected:
		void closeEvent(QCloseEvent* event) override;

	private:
		void edit_item(int row);
		void add_row(const recording_setting &rec_setting);
		void save();
		void set_dirty(bool value);
		bool get_dirty() const;
		void update_new_button();

		QTableWidget m_table_widget;
		QPushButton m_new_button;
		QPushButton m_edit_button;
		QPushButton m_delete_button;

		std::list<recording_setting> m_recording_setting_list;

		bool m_dirty;
};