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

#include "switch.h"
#include <stdlib.h>
#define N(x) scenes_switch_namespace_##x

void N(init)(Scene* sc) { }
void N(render)(Scene* sc) { }
void N(exit)(Scene* sc) { }

SceneResult N(process)(Scene* sc) {
	sc->next_scene = ((Scene*(*)(void))sc->data)();
	return scene_switch;
}

Scene* getSwitchScene(Scene*(*next_scene)(void)) {
	Scene* scene = malloc(sizeof(Scene));
	if (!scene) return NULL;
	scene->init = N(init);
	scene->render = N(render);
	scene->exit = N(exit);
	scene->process = N(process);
	scene->data = (u32)next_scene;
	scene->is_popup = true;
	scene->need_free = true;
	return scene;
}