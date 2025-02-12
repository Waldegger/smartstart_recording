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

#include <obs-frontend-api.h>
#include <obs-module.h> 

#include "smartstart_recording.h"

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("ShivaPlays")
OBS_MODULE_USE_DEFAULT_LOCALE("smartstart_recording", "en-US")

// loading function for plugin
static bool obs_module_load(void) 
{
    return smartstart_recording::get().load();
}

// unload function for plugin
static void obs_module_unload(void) 
{
	smartstart_recording::get().unload();
}