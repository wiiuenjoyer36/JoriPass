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

#include "api.h"
#include "cecd.h"
#include "utils.h"
#include "config.h"
#include "report.h"
#include <stdlib.h>
#include <string.h>

int location = -1;
FS_Archive sharedextdata_b = 0;

Result uploadOutboxes(void) {
	Result res = 0;
	Result messages = 0;
	CecMboxListHeader mbox_list;
	res = cecdOpenAndRead(0, CEC_PATH_MBOX_LIST, sizeof(CecMboxListHeader), (u8*)&mbox_list);
	if (R_FAILED(res)) return -1;
	clearIgnoredTitles(&mbox_list);
	
	{
		char url[50];
		snprintf(url, 50, "%s/outbox/mboxlist", BASE_URL);
		res = httpRequest("POST", url, sizeof(CecMboxListHeader), (u8*)&mbox_list, 0, 0);
		if (R_FAILED(res)) return res;
	}
	for (int i = 0; i < mbox_list.num_boxes; i++) {
		printf("Uploading outbox %d/%ld...", i+1, mbox_list.num_boxes);
		u32 title_id = strtol((const char*)mbox_list.box_names[i], NULL, 16);
		CecBoxInfoFull outbox;
		res = cecdOpenAndRead(title_id, CEC_PATH_OUTBOX_INFO, sizeof(CecBoxInfoFull), (u8*)&outbox);
		if (R_FAILED(res)) continue;
		for (int j = 0; j < outbox.header.num_messages; j++) {
			u8* msg = malloc(MAX(100, outbox.messages[j].message_size));
			if (!msg) {
				printf("ERROR: failed to allocate message\n");
				return -1;
			}
			memset(msg, 0, 100);
			res = cecdOpenAndRead(title_id, CECMESSAGE_BOX_TITLE, 100, msg);
			if (R_FAILED(res)) {
				free(msg);
				continue;
			}
			char* title_name = b64encode(msg, 100);
			res = cecdReadMessage(title_id, true, outbox.messages[j].message_size, msg, outbox.messages[j].message_id);
			if (R_FAILED(res)) {
				free(msg);
				free(title_name);
				continue;
			}
			char url[50];
			if (!validateStreetpassMessage(msg)) {
				snprintf(url, 50, "%s/outbox/%lx", BASE_URL, title_id);
				printf("Deleting ");
				httpRequest("DELETE", url, 0, 0, 0, 0);
				free(msg);
				free(title_name);
				continue;
			}
			snprintf(url, 50, "%s/outbox/upload", BASE_URL);
			CurlReply* reply;
			res = httpRequest("POST", url, outbox.messages[j].message_size, msg, &reply, title_name);
			if (R_FAILED(res)) {
				curlFreeHandler(reply->offset);
				free(msg);
				free(title_name);
				continue;
			}
			if (res == 200) {
				// our reply body has the new send count
				free(msg);
				msg = malloc(outbox.messages[j].message_size);
				if (!msg) {
					free(title_name);
					curlFreeHandler(reply->offset);
					continue;
				}
				res = cecdReadMessage(title_id, true, outbox.messages[j].message_size, msg, outbox.messages[j].message_id);
				if (R_FAILED(res)) {
					free(msg);
					free(title_name);
					curlFreeHandler(reply->offset);
					continue;
				}
				CecMessageHeader* h = (CecMessageHeader*)msg;
				if (reply->ptr[0] < h->send_count) {
					h->send_count = reply->ptr[0];
					res = updateStreetpassOutbox(msg);
					if (R_FAILED(res)) {
						curlFreeHandler(reply->offset);
						free(msg);
						free(title_name);
						continue;
					}
				}
			}
			messages++;
			curlFreeHandler(reply->offset);
			free(msg);
			free(title_name);
		}
		if (R_FAILED(res)) {
			printf("Failed %ld\n", res);
		} else {
			printf("Done\n");
		}
	}
	return messages;
}

Result downloadInboxes(void) {
	Result res = 0;
	Result messages = 0;
	CecMboxListHeader mbox_list;
	CecBoxInfoHeader box_header;
	res = cecdOpenAndRead(0, CEC_PATH_MBOX_LIST, sizeof(CecMboxListHeader), (u8*)&mbox_list);
	if (R_FAILED(res)) return res;
	clearIgnoredTitles(&mbox_list);
	for (int i = 0; i < mbox_list.num_boxes; i++) {
		int title_id = (int)strtol((char*)mbox_list.box_names[i], NULL, 16);
		if (!title_id) continue;
		res = cecdOpenAndRead(title_id, CEC_PATH_INBOX_INFO, sizeof(CecBoxInfoHeader), (u8*)&box_header);
		if (R_FAILED(res)) continue;
		int box_messages = box_header.num_messages;
		printf("Checking inbox %d/%ld", i+1, mbox_list.num_boxes);
		char url[100];
		snprintf(url, 100, "%s/inbox/%s/pop", BASE_URL, mbox_list.box_names[i]);
		u32 http_code = 200;
		while (http_code == 200 && box_messages < box_header.max_num_messages) {
			printf(".");
			CurlReply* reply;
			res = httpRequest("GET", url, 0, 0, &reply, 0);
			if (R_FAILED(res)) break;

			http_code = res;
			if (http_code == 200) {
				res = addStreetpassMessage(reply->ptr);
				if (!R_FAILED(res)) {
					messages++;
					box_messages++;
					saveMsgInLog((CecMessageHeader*)reply->ptr);
				}
			}
			curlFreeHandler(reply->offset);
		}
		if (R_FAILED(res)) {
			printf("Failed %ld\n", res);
		} else {
			printf("Done\n");
		}
	}

	return messages;
}

Result getLocation(void) {
	Result res;
	CurlReply* reply;
	char url[80];
	snprintf(url, 80, "%s/location/current", BASE_URL);
	res = httpRequest("GET", url, 0, 0, &reply, 0);
	if (R_FAILED(res)) goto cleanup;
	int http_code = res;
	if (http_code == 200) {
		res = *(u32*)(reply->ptr);
	} else {
		res = -1;
	}
cleanup:
	curlFreeHandler(reply->offset);
	return res;
}

Result setLocation(int location) {
	Result res;
	if (config.last_location == location) return -1;
	char url[80];
	snprintf(url, 80, "%s/location/%d/enter", BASE_URL, location);
	res = httpRequest("PUT", url, 0, 0, 0, 0);
	if (R_FAILED(res)) {
		printf("ERROR: Failed to enter location %d: %ld\n", location, res);
		return res;
	}
	config.last_location = location;
	configWrite();
	printf("Entered location %d!\n", location);
	return res;
}

void clearIgnoredTitles(CecMboxListHeader* mbox_list) {
	size_t pos = 0;
	for (size_t i = 0; i < mbox_list->num_boxes; i++) {
		u32 title_id = strtol((const char*)mbox_list->box_names[i], NULL, 16);
		if (!isTitleIgnored(title_id)) {
			if (pos != i) memcpy(mbox_list->box_names[pos], mbox_list->box_names[i], 16);
			pos++;
		}
	}
	memset(mbox_list->box_names[pos], 0, 16 * (mbox_list->num_boxes - pos));
	mbox_list->num_boxes = pos;
}

static s32 main_thread_prio_s = 0;
s32 main_thread_prio(void) {
	return main_thread_prio_s;
}
void init_main_thread_prio(void) {
	svcGetThreadPriority(&main_thread_prio_s, CUR_THREAD_HANDLE);
}

static volatile int dl_inbox_status = 1;
static bool dl_loop_running = true;
Thread bg_loop_thread = 0;
void triggerDownloadInboxes(void) {
	while (dl_inbox_status != 0) svcSleepThread((u64)1000000 * 100);
	dl_inbox_status = 1;
}

void bgLoop(void* p) {
	do {
		dl_inbox_status = 2;
		downloadInboxes();
		dl_inbox_status = 0;
		for(int i = 0; i < 10*60*5; i++) {
			svcSleepThread((u64)1000000 * 100);
			if (dl_inbox_status == 1 || !dl_loop_running) break;
		}
	} while(dl_loop_running);
}

void bgLoopInit(void) {
	bg_loop_thread = threadCreate(bgLoop, NULL, 8*1024, main_thread_prio()-1, -2, false);
}

void bgLoopExit(void) {
	dl_loop_running = false;
	if (bg_loop_thread) {
		threadFree(bg_loop_thread);
	}
}
