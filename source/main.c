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

#include <3ds.h>
#include <citro2d.h>
#include <stdlib.h>
#include <stdio.h> // from soundexample
#include <string.h> // from soundexample
#include "scene.h"
#include "api.h"
#include "cecd.h"
#include "curl-handler.h"
#include "config.h"

// stuff from soundexample
u8* buffer;
u32 size;

// more soundexample stuff
static void audio_load(const char *audio);
static void audio_stop(void);

int main() {
	gfxInitDefault();
	cfguInit();
	amInit();
	nsInit();
	srvInit(); // from soundexample
	hidInit(); // from soundexample
	bool stop = 0 // from soundexample
	aptInit();
	consoleInit(GFX_BOTTOM, NULL);
	printf("    #              ###                 \n    #   ##  ###  # #  #   #    ##   ##\n    #  #  # #  # # #  #  # #  #    #   \n    #  #  # #  # # ###  #   #  ##   ## \n #  #  #  # ###  # #    #####    #    #\n #  #  #  # # #  # #    #   # #  # #  #\n  ##    ##  #  # # #    #   #  ##   ## \nStarting JoriPass v%d.%d.%d\n", _VERSION_MAJOR_, _VERSION_MINOR_, _VERSION_MICRO_);
	stringsInit();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	romfsInit();
	init_main_thread_prio();

	cecdInit();
	curlInit();
	srand(time(NULL));

	configInit(); // must be after cecdInit()

	csndInit(); // start audio lib, from soundexample
	u32 kDown; // from soundexample
	// mount sharedextdata_b so that we can read it later, for e.g. playcoins
	{
		u32 extdata_lowpathdata[3];
		memset(extdata_lowpathdata, 0, 0xc);
		extdata_lowpathdata[0] = MEDIATYPE_NAND;
		extdata_lowpathdata[1] = 0xf000000b;
		FS_Path extdata_path = {
			type: PATH_BINARY,
			size: 0xC,
			data: (u8*)extdata_lowpathdata,
		};
		archiveMount(ARCHIVE_SHARED_EXTDATA, extdata_path, "sharedextdata_b");
		FSUSER_OpenArchive(&sharedextdata_b, ARCHIVE_SHARED_EXTDATA, extdata_path);
	}

	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	Scene* scene = getLoadingScene(getSwitchScene(lambda(Scene*, (void) {
		if (R_FAILED(location) && location != -1) {
			// something not working
			return getConnectionErrorScene(location);
		}
		bgLoopInit();
		if (location == -1) {
			return getHomeScene(); // load home
		}
		return getLocationScene(location);
	})), lambda(void, (void) {
		Result res;
		// first, we gotta wait for having internet
		char url[50];
		snprintf(url, 50, "%s/ping", BASE_URL);
	check_internet:
		res = httpRequest("GET", url, 0, 0, 0, 0);
		if (R_FAILED(res)) {
			if (res == -CURLE_COULDNT_RESOLVE_HOST) {
				svcSleepThread((u64)1000000 * 100);
				goto check_internet;
			}
			location = res;
			return;
		}
		uploadOutboxes();
		res = getLocation();
		if (R_FAILED(res) && res != -1) {
			printf("ERROR failed to get location: %ld\n", res);
			location = -1;
		} else {
			location = res;
			if (location == -1) {
				printf("Got location home\n");
			} else {
				printf("Got location: %d\n", location);
			}
		}
	}));

	scene->init(scene);

audio_load("audio/original_raw.bin");

	while (aptMainLoop()) {
		Scene* new_scene = processScene(scene);
		if (!new_scene) break;
		if (new_scene != scene) {
			scene = new_scene;
			continue;
		}
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF));
		C2D_SceneBegin(top);
		if (scene->is_popup) {
			scene->pop_scene->render(scene->pop_scene);
			C2D_Flush();
		}
		scene->render(scene);
		C3D_FrameEnd(0);
		svcSleepThread(1);
	}
	printf("Exiting...\n");
	bgLoopExit();
	C2D_Fini();
	C3D_Fini();
	//curlExit();
	romfsExit();
	aptExit();
	nsExit();
	amExit();
	cfguExit();
	gfxExit();
	return 0;
}

void audio_load(const char *audio){

	FILE *file = fopen(audio, "rb");
	fseek(file, 0, SEEK_END);
	off_t size = ftell(file);
	fseek(file, 0, SEEK_SET);
	buffer = linearAlloc(size);
	off_t bytesRead = fread(buffer, 1, size, file);
	fclose(file);
	csndPlaySound(8, SOUND_FORMAT_16BIT | SOUND_REPEAT, 48000, 1, 0, buffer, buffer, size);
	linearFree(buffer);
}
void audio_stop(void){
	csndExecCmds(true);
	CSND_SetPlayState(0x8, 0);
	memset(buffer, 0, size);
	GSPGPU_FlushDataCache(buffer, size);
	linearFree(buffer);
}
