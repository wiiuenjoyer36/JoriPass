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

#include "connection_error.h"
#include <stdlib.h>
#include <curl/curl.h>
#define N(x) scenes_error_namespace_##x
#define _data ((N(DataStruct)*)sc->d)

typedef struct {
	C2D_TextBuf g_staticBuf;
	C2D_Text g_title;
	C2D_Text g_subtext;
} N(DataStruct);

void N(init)(Scene* sc) {
	sc->d = malloc(sizeof(N(DataStruct)));
	if (!_data) return;
	_data->g_staticBuf = C2D_TextBufNew(1000);
	Result res = (Result)sc->data;
	char str[200];
	const char* subtext = "";
	C2D_Font str_font;
	C2D_Font subtext_font = _font(str_libcurl_error);
	if (res > -100) {
		// libcurl error code
		int errcode = -res;
		const char* errmsg = curl_easy_strerror(errcode);
		snprintf(str, 200, _s(str_libcurl_error), errcode, errmsg);
		str_font = _font(str_libcurl_error);
		if (errcode == 60) {
			subtext = _s(str_libcurl_date_and_time);
			subtext_font = _font(str_libcurl_date_and_time);
		}
	} else if (res > -600) {
		// http status code
		int status_code = -res;
		snprintf(str, 200, _s(str_httpstatus_error), status_code);
		str_font = _font(str_httpstatus_error);
	} else {
		// 3ds error code
		snprintf(str, 200, _s(str_3ds_error), (u32)res);
		str_font = _font(str_3ds_error);
	}
	C2D_TextFontParse(&_data->g_title, str_font, _data->g_staticBuf, str);
	C2D_TextFontParse(&_data->g_subtext, subtext_font, _data->g_staticBuf, subtext);
}

void N(render)(Scene* sc) {
	if (!_data) return;
	C2D_DrawText(&_data->g_title, C2D_AlignLeft, 10, 10, 0, 0.5, 0.5);
	C2D_DrawText(&_data->g_subtext, C2D_AlignLeft, 10, 35, 0, 0.5, 0.5);
}

void N(exit)(Scene* sc) {
	if (_data) {
		C2D_TextBufDelete(_data->g_staticBuf);
		free(_data);
	}
}

SceneResult N(process)(Scene* sc) {
	hidScanInput();
	u32 kDown = hidKeysDown();
	if (kDown & KEY_START) return scene_stop;
	return scene_continue;
}

Scene* getConnectionErrorScene(Result res) {
	Scene* scene = malloc(sizeof(Scene));
	if (!scene) return NULL;
	scene->init = N(init);
	scene->render = N(render);
	scene->exit = N(exit);
	scene->process = N(process);
	scene->data = (u32)res;
	scene->is_popup = false;
	scene->need_free = true;
	return scene;
}