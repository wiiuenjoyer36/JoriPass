/**
 * NetPass
 * Copyright (C) 2024 Sorunome
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <3ds.h>
#include <citro2d.h>
#include "api.h"
#include "strings.h"
#include "config.h"

#define SCREEN_TOP_WIDTH 400
#define SCREEN_TOP_HEIGHT 240

typedef enum {scene_stop, scene_continue, scene_switch, scene_push, scene_pop} SceneResult;

typedef struct Scene Scene;

struct Scene {
	void (*init)(Scene*);
	void (*render)(Scene*);
	void (*exit)(Scene*);
	SceneResult (*process)(Scene*);
	Scene* next_scene;
	Scene* pop_scene;
	u32 data;

	void* d;
	bool is_popup;
	bool need_free;
};

Scene* processScene(Scene* scene);

#include "scenes/loading.h"
#include "scenes/switch.h"
#include "scenes/home.h"
#include "scenes/location.h"
#include "scenes/connection_error.h"
#include "scenes/settings.h"
#include "scenes/info.h"
#include "scenes/report_list.h"
#include "scenes/back_alley.h"
#include "scenes/toggle_titles.h"