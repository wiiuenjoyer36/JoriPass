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
#include "../api.h"
#include <stdlib.h>
#define N(x) scenes_location_namespace_##x
#define _data ((N(DataStruct)*)sc->d)

typedef struct {
	C2D_TextBuf g_staticBuf;
	C2D_Text g_location;
	C2D_Text g_entries[4];
	C2D_SpriteSheet spr;
	int cursor;
} N(DataStruct);

LanguageString* N(locations)[NUM_LOCATIONS] = {
	&str_at_train_station,
	&str_at_plaza,
	&str_at_mall,
	&str_at_beach,
	&str_at_arcade,
	&str_at_catcafe,
};

void N(init)(Scene* sc) {
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data) return;
	_data->g_staticBuf = C2D_TextBufNew(2000);
	_data->cursor = 0;
	TextLangParse(&_data->g_location, _data->g_staticBuf, *N(locations)[sc->data]);
	TextLangParse(&_data->g_entries[0], _data->g_staticBuf, str_check_inboxes);
	TextLangParse(&_data->g_entries[1], _data->g_staticBuf, str_back_alley);
	TextLangParse(&_data->g_entries[2], _data->g_staticBuf, str_settings);
	TextLangParse(&_data->g_entries[3], _data->g_staticBuf, str_exit);
	_data->spr = C2D_SpriteSheetLoad("romfs:/gfx/locations.t3x");
}

void N(render)(Scene* sc) {
	if (!_data) return;
	if (C2D_SpriteSheetCount(_data->spr) > sc->data) {
		C2D_Image img = C2D_SpriteSheetGetImage(_data->spr, sc->data);
		C2D_DrawImageAt(img, 0, 0, 0, NULL, 1, 1);
	}
	u32 bgclr = C2D_Color32(0, 0, 0, 0x50);
	float width;
	get_text_dimensions(&_data->g_location, 1, 1, &width, 0);
	C2D_DrawRectSolid(8, 8, 0, width + 4, 10 + 5*25, bgclr);
	u32 clr = C2D_Color32(0xff, 0xff, 0xff, 0xff);
	C2D_DrawText(&_data->g_location, C2D_AlignLeft | C2D_WithColor, 10, 10, 0, 1, 1, clr);
	for (int i = 0; i < 4; i++) {
		C2D_DrawText(&_data->g_entries[i], C2D_AlignLeft | C2D_WithColor, 30, 10 + (i+1)*25, 0, 1, 1, clr);
	}
	int x = 10;
	int y = 10 + (_data->cursor + 1)*25 + 5;
	C2D_DrawTriangle(x, y, clr, x, y + 18, clr, x + 15, y + 9, clr, 1);
}

void N(exit)(Scene* sc) {
	if (_data) {
		C2D_TextBufDelete(_data->g_staticBuf);
		C2D_SpriteSheetFree(_data->spr);
		free(_data);
	}
}

SceneResult N(process)(Scene* sc) {
	hidScanInput();
	u32 kDown = hidKeysDown();
	if (_data) {
		_data->cursor += ((kDown & KEY_DOWN || kDown & KEY_CPAD_DOWN) && 1) - ((kDown & KEY_UP || kDown & KEY_CPAD_UP) && 1);
		if (_data->cursor < 0) _data->cursor = 2;
		if (_data->cursor > 2) _data->cursor = 0;
		if (kDown & KEY_A) {
			if (_data->cursor == 0) {
				sc->next_scene = getLoadingScene(0, lambda(void, (void) {
					downloadInboxes();
				}));
				return scene_push;
			}
			if (_data->cursor == 1) {
				sc->next_scene = getBackAlleyScene();
				return scene_push;
			}
			if (_data->cursor == 2) {
				sc->next_scene = getSettingsScene();
				return scene_push;
			}
			if (_data->cursor == 3) return scene_stop;
		}
	}
	if (kDown & KEY_START) return scene_stop;
	return scene_continue;
}

Scene* getLocationScene(int location) {
	Scene* scene = malloc(sizeof(Scene));
	if (!scene) return NULL;
	scene->init = N(init);
	scene->render = N(render);
	scene->exit = N(exit);
	scene->process = N(process);
	scene->data = location;
	scene->is_popup = false;
	scene->need_free = true;
	return scene;
}
