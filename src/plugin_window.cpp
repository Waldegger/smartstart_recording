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

#include "plugin_window.h"

#include <QLabel>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QVariant>
#include <QCloseEvent>
#include <QMessageBox>

#include <sstream>
#include <memory>

#include <obs-module.h>

#include "constants.h"
#include "record_edit_window.h"
#include "smartstart_recording.h"

plugin_window::plugin_window(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow{ parent, flags }
	, m_table_widget{ 0, 3, this }
	, m_new_button{ this }
	, m_edit_button{ this }
	, m_delete_button{ this }
	, m_dialog_button_box{ QDialogButtonBox::StandardButton::Save | QDialogButtonBox::Apply | QDialogButtonBox::StandardButton::Close, this  }
	, m_dirty{ false }
{
	setWindowTitle(PLUGIN_NAME.data());
	setFixedSize(800, 450);
	
	auto group_box = new QGroupBox{ this };
	group_box->setTitle(obs_module_text("recording_settings_per_scene"));
	
	m_table_widget.verticalHeader()->hide();
	m_table_widget.setHorizontalHeaderLabels(QStringList() << obs_module_text("table_widget.scene") << obs_module_text("table_widget.recording") << obs_module_text("table_widget.time_in_ms"));
	m_table_widget.setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table_widget.setSelectionMode(QAbstractItemView::SingleSelection);
	m_table_widget.horizontalHeader()->setHighlightSections(false);
	m_table_widget.setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table_widget.setFocusPolicy(Qt::FocusPolicy::NoFocus);
	m_table_widget.setColumnWidth(0, 350);
	m_table_widget.setColumnWidth(1, 100);
	m_table_widget.setColumnWidth(2, 150);
	m_recording_setting_list = smartstart_recording::get().get_recording_setting_list();
	
	for (auto& v : m_recording_setting_list)
		add_row(v);
	
	auto table_widget_cell_double_click = [this](int row, int column) -> void
		{
			edit_item(row);
		};

	auto table_widget_selection_changed = [this]() -> void
		{
			if (!m_table_widget.selectedItems().count())
			{
				//disable buttons
				m_edit_button.setEnabled(false);
				m_delete_button.setEnabled(false);
				return;
			}

			m_edit_button.setEnabled(true);
			m_delete_button.setEnabled(true);
		};

	auto new_button_click = [this]() -> void 
		{
			auto edit_window = new record_edit_window{ m_recording_setting_list, this };
			auto dlg_finished = [this, edit_window](int result) -> void
				{
					if (result == QDialog::Rejected)
						return;

					auto& item = edit_window->get_recording_setting().value();
					m_recording_setting_list.push_back(item);
			
					add_row(m_recording_setting_list.back());
					set_dirty(true);

					update_new_button();
				};

			edit_window->setAttribute(Qt::WA_DeleteOnClose);
			edit_window->open();
			connect(edit_window, &QDialog::finished, dlg_finished);
		};

	auto edit_button_click = [this]() -> void
		{
			edit_item(m_table_widget.currentRow());
		};

	auto delete_button_click = [this]() -> void
		{
			auto row = m_table_widget.currentRow();
			auto& item = *reinterpret_cast<recording_setting*>(qvariant_cast<std::uintptr_t>(m_table_widget.item(row, 0)->data(Qt::UserRole)));

			m_recording_setting_list.remove(item);
			m_table_widget.removeRow(row);

			set_dirty(true);

			m_new_button.setEnabled(true);
		};

	auto apply_button_click = [this]() -> void
		{
			save();
		};

	auto save_button_click = [this]() -> void
		{
			save();
			close();
		};

	auto close_button_click = [this]() -> void
		{
			close();
		};

	auto vbox = new QVBoxLayout{ this };
	auto table_edit_layout = new QHBoxLayout{ this };
	auto table_layout = new QHBoxLayout{ this };
	auto button_layout = new QVBoxLayout{ this };
	auto spacer_layout = new QHBoxLayout(this);
	auto bottom_button_layout = new QHBoxLayout{ this };
	
	connect(&m_table_widget, &QTableWidget::cellDoubleClicked, table_widget_cell_double_click);
	connect(&m_table_widget, &QTableWidget::itemSelectionChanged, table_widget_selection_changed);

	connect(&m_new_button, &QPushButton::pressed, new_button_click);
	connect(&m_edit_button, &QPushButton::pressed, edit_button_click);
	connect(&m_delete_button, &QPushButton::pressed, delete_button_click);

	connect(m_dialog_button_box.button(QDialogButtonBox::StandardButton::Save), &QPushButton::pressed, save_button_click);
	connect(m_dialog_button_box.button(QDialogButtonBox::StandardButton::Apply), &QPushButton::pressed, apply_button_click);
	connect(m_dialog_button_box.button(QDialogButtonBox::StandardButton::Close), &QPushButton::pressed, close_button_click);

	update_new_button();
	m_new_button.setText(obs_module_text("button.new"));
	m_new_button.setMinimumWidth(150);
	m_edit_button.setText(obs_module_text("button.edit"));
	m_edit_button.setMinimumWidth(150);
	m_edit_button.setEnabled(false);
	m_delete_button.setText(obs_module_text("button.delete"));
	m_delete_button.setMinimumWidth(150);
	m_delete_button.setEnabled(false);
	m_dialog_button_box.button(QDialogButtonBox::StandardButton::Apply)->setEnabled(false);

	table_layout->addWidget(&m_table_widget);
	table_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

	table_edit_layout->addLayout(table_layout);
	table_edit_layout->addLayout(button_layout);

	button_layout->setSpacing(0);
	button_layout->addWidget(&m_new_button);
	button_layout->addWidget(&m_edit_button);
	button_layout->addSpacing(50);
	button_layout->addWidget(&m_delete_button);
	button_layout->setAlignment(Qt::AlignTop);

	bottom_button_layout->addWidget(&m_dialog_button_box);

	vbox->addLayout(table_edit_layout);

	auto spacer_line = new QFrame(this);
	spacer_line->setFrameShape(QFrame::HLine);
	spacer_line->setFrameShadow(QFrame::Sunken);
	spacer_layout->addWidget(spacer_line);
	vbox->addLayout(spacer_layout); 
	vbox->addLayout(bottom_button_layout);
	group_box->setLayout(vbox);
	group_box->setFlat(true);

	setCentralWidget(group_box);
}

void plugin_window::closeEvent(QCloseEvent* event)
{
	QMainWindow::closeEvent(event);

	if (m_dirty)
	{
		auto result = QMessageBox::question(this, obs_module_text("msgbox_unsaved.title"), obs_module_text("msgbox_unsaved.text"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

		switch (result)
		{
			case QMessageBox::Save:
			{
				save();
				event->accept();
			}
			break;
			case QMessageBox::Discard:
			{
				event->accept();
			}
			break;
			default:
			{
				event->ignore();
			}
			break;
		}
		
		return;
	}

	event->accept();
}

void plugin_window::edit_item(int row)
{
	auto& item = *reinterpret_cast<recording_setting*>(qvariant_cast<std::uintptr_t>(m_table_widget.item(row, 0)->data(Qt::UserRole)));

	auto edit_window = new record_edit_window{ m_recording_setting_list, this };
	auto dlg_finished = [this, edit_window, &item, row](int result) -> void
		{
			if (result == QDialog::Rejected)
				return;

			if (item != edit_window->get_recording_setting().value())
			{
				item = edit_window->get_recording_setting().value();

				m_table_widget.item(row, 0)->setText(item.get_scene_name().c_str());
				m_table_widget.item(row, 1)->setText(item.get_action() == recording_setting::action::start ? obs_module_text("start") : obs_module_text("stop"));
				m_table_widget.item(row, 2)->setText(std::to_string(item.get_trigger_time()).c_str());

				set_dirty(true);
			}
		};

	edit_window->setAttribute(Qt::WA_DeleteOnClose);
	edit_window->set_recording_setting(item);
	edit_window->open();
	connect(edit_window, &QDialog::finished, dlg_finished);
}

void plugin_window::add_row(const recording_setting& rec_setting)
{
	auto i = m_table_widget.rowCount();
	m_table_widget.insertRow(i);

	auto data = reinterpret_cast<std::uintptr_t>(&rec_setting);
	auto scene_name_item = new QTableWidgetItem(rec_setting.get_scene_name().c_str());
	scene_name_item->setData(Qt::UserRole, data);
	m_table_widget.setItem(i, 0, scene_name_item);

	auto action_item = new QTableWidgetItem(rec_setting.get_action() == recording_setting::action::start ? obs_module_text("start") : obs_module_text("stop"));
	action_item->setTextAlignment(Qt::AlignCenter);
	m_table_widget.setItem(i, 1, action_item);

	auto trigger_time_item = new QTableWidgetItem(std::to_string(rec_setting.get_trigger_time()).c_str());
	trigger_time_item->setTextAlignment(Qt::AlignRight);
	m_table_widget.setItem(i, 2, trigger_time_item);
}

void plugin_window::save()
{
	if (m_dirty)
	{
		smartstart_recording::get().update_recording_settings(m_recording_setting_list);
		set_dirty(false);
	}
}

void plugin_window::set_dirty(bool value)
{
	if (m_dirty == value) return;

	m_dirty = value;

	if (m_dirty)
		setWindowTitle("*" + windowTitle());
	else
		setWindowTitle(windowTitle().mid(1));

	m_dialog_button_box.button(QDialogButtonBox::StandardButton::Apply)->setEnabled(m_dirty);
}

bool plugin_window::get_dirty() const
{
	return m_dirty;
}

void plugin_window::update_new_button()
{
	auto scene_list = std::unique_ptr<char*, std::function<void(char**)>>(obs_frontend_get_scene_names(), [](char** ptr)->void { bfree(ptr); });
	m_new_button.setEnabled(false);

	for (size_t i = 0; scene_list.get()[i]; ++i)
	{
		auto item = scene_list.get()[i];

		bool item_found = false;
		for (auto& v : m_recording_setting_list)
		{
			if (item == v.get_scene_name())
			{
				item_found = true;
				break;
			}
		}

		if (!item_found)
		{
			m_new_button.setEnabled(true);
			return;
		}
	}
}