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

#include "report.h"
#include "config.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>

#define LOG_DIR "/config/netpass/log/"
#define LOG_INDEX "/config/netpass/log/index.nrle"

#define MAX_REPORT_ENTRIES_LEN 128

ReportList* loadReportList(void) {
	FILE* f = fopen(LOG_INDEX, "rb");
	if (!f) return NULL;

	ReportListHeader header;
	fread(&header, sizeof(ReportListHeader), 1, f);
	if (header.magic != 0x454C524e || header.version != 1) return NULL;
	fseek(f, 0, SEEK_SET);
	size_t list_file_size = sizeof(ReportListHeader) + header.max_size * sizeof(ReportSendPayload);
	
	ReportList* list = memalign(4, list_file_size);
	if (!list) return NULL;
	fread(list, list_file_size, 1, f);
	fclose(f);
	return list;
}

void saveMsgInLog(CecMessageHeader* msg) {
	ReportList* list;
	mkdir_p(LOG_DIR);
	FILE* f = fopen(LOG_INDEX, "rb");
	if (!f) {
		// ok, file is empty, we have to create it
		f = fopen(LOG_INDEX, "wb");
		if (!f) return;
		list = memalign(4, sizeof(ReportListHeader) + sizeof(ReportListEntry) * MAX_REPORT_ENTRIES_LEN);
		if (!list) return;
		memset(list, 0, sizeof(ReportListHeader) + sizeof(ReportListEntry) * MAX_REPORT_ENTRIES_LEN);
		list->header.magic = 0x454C524e;
		list->header.version = 1;
		list->header.max_size = MAX_REPORT_ENTRIES_LEN;
		list->header.cur_size = 0;
		fwrite(list, sizeof(ReportList), 1, f);
		free(list);
		fclose(f);
		f = fopen(LOG_INDEX, "rb");
		if (!f) return;
	}
	size_t list_file_size;
	{
		ReportListHeader header;
		fread(&header, sizeof(ReportListHeader), 1, f);
		if (header.magic != 0x454C524e || header.version != 1) return;
		fseek(f, 0, SEEK_SET);
		list_file_size = sizeof(ReportListHeader) + header.max_size * sizeof(ReportSendPayload);
		list = memalign(4, list_file_size);
		if (!list) return;
		fread(list, list_file_size, 1, f);
		fclose(f);
	}

	int found_i = -1;
	// find if the batch already exists
	for (int i = 0; i < list->header.cur_size; i++) {
		if (list->entries[i].batch_id == msg->batch_id) {
			found_i = i;
			break;
		}
	}
	char* b64name = b64encode(msg->message_id, 8);
	char filename[100];
	snprintf(filename, 100, "%s%lx/_%s", LOG_DIR, msg->batch_id, b64name);
	free(b64name);
	bool edited = false;
	if (found_i < 0) {
		// we have to add a new entry!
		if (list->header.max_size == list->header.cur_size) {
			// uho, all is full, gotta pop the first one of the list
			u32 rm_batch = list->entries[0].batch_id;
			char rm_dirname[100];
			snprintf(rm_dirname, 100, "%s%lx", LOG_DIR, rm_batch);
			rmdir_r(rm_dirname);
			list->header.cur_size--;
			memmove(list->entries, ((u8*)list->entries) + sizeof(ReportListEntry), list->header.cur_size * sizeof(ReportListEntry));
		}
		ReportListEntry* e = &list->entries[list->header.cur_size];
		e->batch_id = msg->batch_id;
		memcpy(&e->received, &msg->received, sizeof(CecTimestamp));
		found_i = list->header.cur_size;
		list->header.cur_size++;
		edited = true;
	}
	ReportListEntry* e = &list->entries[found_i];
	if (msg->title_id == 0x20800) {
		// mii plaza
		static const int cfpb_offset = 0x36bc;
		static const int cfpb_size = 0x88;
		if (msg->message_size > msg->total_header_size + cfpb_offset + cfpb_size) {
			CFPB* cfpb = (CFPB*)((u8*)msg + msg->total_header_size + cfpb_offset);
			if (cfpb->magic == 0x42504643) {
				int prev_mii_id = e->mii.magic == 3 ? e->mii.mii_id : 0;
				Result r = decryptMii(&cfpb->nonce, &e->mii);
				if (!R_FAILED(r) && prev_mii_id != e->mii.mii_id) edited = true;
			}
		}
	} else if (e->mii.magic != 3) {
		// search if there is a mii in this payload
		CFPB* cfpb = (CFPB*)memsearch((u8*)msg, msg->message_size, (u8*)"CFPB", 4);
		if (cfpb) {
			Result r = decryptMii(&cfpb->nonce, &e->mii);
			if (!R_FAILED(r)) edited = true;
		}
	}

	if (edited) {
		f = fopen(LOG_INDEX, "wb");
		if (!f) goto error;
		fwrite(list, list_file_size, 1, f);
		fclose(f);
	}
	mkdir_p(filename);
	f = fopen(filename, "wb");
	if (!f) goto error;
	fwrite(msg, msg->message_size, 1, f);
	fclose(f);

error:
	free(list);
}

Result reportGetSomeMsgHeader(CecMessageHeader* msg, u32 batch_id) {
	msg->magic = 0;

	char dirname[100];
	snprintf(dirname, 100, "%s%lx", LOG_DIR, batch_id);
	size_t path_len = strlen(dirname);

	DIR* d = opendir(dirname);
	if (!d) return -1;

	struct dirent *p;
	int r = 0;
	while (!r && (p=readdir(d))) {
		int fname_len = path_len + strlen(p->d_name) + 2;
		char* fname = malloc(fname_len);
		snprintf(fname, fname_len, "%s/%s", dirname, p->d_name);
		// we found a file
		FILE* f = fopen(fname, "rb");
		if (f) {
			fread(msg, sizeof(CecMessageHeader), 1, f);
			fclose(f);
			if (msg->magic == 0x6060 && msg->batch_id == batch_id) {
				free(fname);
				break;
			} else {
				msg->magic = 0;
			}
		}
		free(fname);
	}

	if (!msg->magic) return -2; // nothing in directory

	return 0;
}