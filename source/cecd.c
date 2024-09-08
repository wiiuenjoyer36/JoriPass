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

#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include "cecd.h"
#include <3ds/ipc.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static Handle cecdHandle;
static int cecdRefCount;

void getCurrentTime(CecTimestamp* cts) {
	time_t unix_time = time(NULL);
	struct tm* ts = gmtime((const time_t*)&unix_time);
	cts->hour = ts->tm_hour;
	cts->minute = ts->tm_min;
	cts->second = ts->tm_sec;
	cts->millisecond = 0;
	cts->year = ts->tm_year + 1900;
	cts->month = ts->tm_mon + 1;
	cts->day = ts->tm_mday;
	cts->weekday = ts->tm_wday;
}

Result cecdInit(void) {
	Result res = 0;

	if (AtomicPostIncrement(&cecdRefCount)) return 0;

	res = srvGetServiceHandle(&cecdHandle, "cecd:s");
	if (R_FAILED(res)) {
		res = srvGetServiceHandle(&cecdHandle, "cecd:u");
	}
	if (R_FAILED(res)) goto cleanup;

	return res;
cleanup:
	AtomicDecrement(&cecdRefCount);
	return res;
}

Result cecdGetState(u32* state) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x0E, 0, 0);
	
	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	*state = cmdbuf[2];

	return res;
}

Result cecdReadMessage(u32 program_id, bool is_outbox, u32 size, u8* buf, CecMessageId message_id) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x03, 4, 4);
	cmdbuf[1] = program_id;
	cmdbuf[2] = (u32)is_outbox;
	cmdbuf[3] = 8; // message id size
	cmdbuf[4] = size;

	cmdbuf[5] = IPC_Desc_Buffer(8, IPC_BUFFER_R);
	cmdbuf[6] = (u32)message_id;
	cmdbuf[7] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[8] = (u32)buf;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdReadMessageWithHMAC(u32 program_id, bool is_outbox, u32 size, u8* buf, CecMessageId message_id, u8* hmac) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x03, 4, 4);
	cmdbuf[1] = program_id;
	cmdbuf[2] = (u32)is_outbox;
	cmdbuf[3] = 8; // message id size
	cmdbuf[4] = size;

	cmdbuf[5] = IPC_Desc_Buffer(8, IPC_BUFFER_R);
	cmdbuf[6] = (u32)message_id;
	cmdbuf[7] = IPC_Desc_Buffer(32, IPC_BUFFER_R);
	cmdbuf[8] = (u32)hmac;
	cmdbuf[9] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[10] = (u32)buf;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdWriteMessage(u32 program_id, bool is_outbox, u32 size, u8* buf, CecMessageId message_id) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x06, 4, 4);
	cmdbuf[1] = program_id;
	cmdbuf[2] = (u32)is_outbox;
	cmdbuf[3] = 8; // message id size
	cmdbuf[4] = size;

	cmdbuf[5] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[6] = (u32)buf;
	cmdbuf[7] = IPC_Desc_Buffer(8, IPC_BUFFER_RW);
	cmdbuf[8] = (u32)message_id;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdWriteMessageWithHMAC(u32 program_id, bool is_outbox, u32 size, u8* buf, CecMessageId message_id, u8* hmac) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x07, 4, 6);
	cmdbuf[1] = program_id;
	cmdbuf[2] = (u32)is_outbox;
	cmdbuf[3] = 8; // message id size
	cmdbuf[4] = size;

	cmdbuf[5] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[6] = (u32)buf;
	cmdbuf[7] = IPC_Desc_Buffer(32, IPC_BUFFER_R);
	cmdbuf[8] = (u32)hmac;
	cmdbuf[9] = IPC_Desc_Buffer(8, IPC_BUFFER_RW);
	cmdbuf[10] = (u32)message_id;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdGetCecInfoEventHandle(Handle* handle) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xF, 4, 4);
	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];
	*handle = cmdbuf[3];

	return res;
}

Result cecdGetChangeStateEventHandle(Handle* handle) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x10, 4, 4);
	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];
	*handle = cmdbuf[3];

	return res;
}

Result cecdOpenAndWrite(u32 program_id, u32 path_type, u32 size, u8* buf) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x11, 4, 4);
	cmdbuf[1] = size;
	cmdbuf[2] = program_id;
	cmdbuf[3] = path_type;
	cmdbuf[4] = 0;

	cmdbuf[5] = IPC_Desc_CurProcessId();
	cmdbuf[6] = 0;
	cmdbuf[7] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[8] = (u32)buf;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdOpenAndRead(u32 program_id, u32 path_type, u32 size, u8* buf) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x12, 4, 4);
	cmdbuf[1] = size;
	cmdbuf[2] = program_id;
	cmdbuf[3] = path_type;
	cmdbuf[4] = 0;

	cmdbuf[5] = IPC_Desc_CurProcessId();
	cmdbuf[6] = 0;
	cmdbuf[7] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[8] = (u32)buf;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

// this method is broken
Result cecdGetSystemInfo(u32 destbuf_size, void* destbuf) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u8 parambuf[32];
	cmdbuf[0] = IPC_MakeHeader(0x0A, 3, 4); // 0x000A00C4
	cmdbuf[1] = 32; // dest buffer size
	cmdbuf[2] = 1; // info type
	cmdbuf[3] = 32; // param buffer size

	cmdbuf[4] = IPC_Desc_Buffer(32, IPC_BUFFER_R);
	cmdbuf[5] = (u32)parambuf;
	cmdbuf[4] = IPC_Desc_Buffer(destbuf_size, IPC_BUFFER_W);
	cmdbuf[5] = (u32)destbuf;

	
	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Handle cecdGetServHandle(void) {
	return cecdHandle;
}

Result updateStreetpassOutbox(u8* msgbuf) {
	Result res = 0;
	CecMessageHeader* msgheader = (CecMessageHeader*)msgbuf;
	// sanity checks
	if (msgheader->magic != 0x6060) return -1; // bad magic
	if (msgheader->message_size != msgheader->total_header_size + msgheader->body_size + 0x20) return -1;
	if (msgheader->message_size > MAX_MESSAGE_SIZE) return -1; // prooobably too large

	// first fetch how large the boxbuf is
	u8* boxbuf = malloc(sizeof(CecBoxInfoHeader));
	if (!boxbuf) {
		return -3;
	}
	res = cecdOpenAndRead(msgheader->title_id, CEC_PATH_OUTBOX_INFO, sizeof(CecBoxInfoHeader), boxbuf);
	if (R_FAILED(res)) {
		res = -2; // cecd fild not found
		goto cleanup_box;
	}
	// let's open the box buffer to update the metadata in there
	int max_boxbuf_size = sizeof(CecBoxInfoHeader) + sizeof(CecMessageHeader) * ((CecBoxInfoHeader*)boxbuf)->max_num_messages;
	free(boxbuf);
	boxbuf = malloc(max_boxbuf_size);
	if (!boxbuf) {
		return -3;
	}
	res = cecdOpenAndRead(msgheader->title_id, CEC_PATH_OUTBOX_INFO, max_boxbuf_size, boxbuf);
	if (R_FAILED(res)) {
		res = -2; // cecd fild not found
		goto cleanup_box;
	}
	CecBoxInfoHeader* boxheader = (CecBoxInfoHeader*)boxbuf;
	CecMessageHeader* boxmsgs = (CecMessageHeader*)(boxbuf + sizeof(CecBoxInfoHeader));

	int found_i = -1;
	for (int i = 0; i < boxheader->num_messages; i++) {
		if (0 == memcmp(boxmsgs[i].message_id, msgheader->message_id, sizeof(CecMessageId))){
			found_i = i;
			break;
		}
	}
	if (found_i < 0) {
		res = -4; // not found
		goto cleanup_box;
	}
	memcpy(&boxmsgs[found_i], msgheader, sizeof(CecMessageHeader));

	res = cecdOpenAndWrite(msgheader->title_id, CEC_PATH_OUTBOX_INFO, boxheader->file_size, boxbuf);
	if (R_FAILED(res)) {
		res = -2; // cecd fild not found
		goto cleanup_box;
	}
	free(boxbuf);

	// now let's fetch the hmac and store the update
	CecMBoxInfoHeader mboxheader;
	res = cecdOpenAndRead(msgheader->title_id, CEC_PATH_MBOX_INFO, sizeof(CecMBoxInfoHeader), (u8*)&mboxheader);
	if (R_FAILED(res)) return -2;

	// great,we have all the bits we need now
	res = cecdWriteMessageWithHMAC(
		msgheader->title_id, true,
		msgheader->message_size, msgbuf,
		msgheader->message_id, mboxheader.hmac_key);
	if (R_FAILED(res)) return res;

	return res;
cleanup_box:
	free(boxbuf);
	return res;
}

bool validateStreetpassMessage(u8* msgbuf) {
	CecMessageHeader* msgheader = (CecMessageHeader*)msgbuf;
	// sanity checks
	if (msgheader->magic != 0x6060) return false; // bad magic
	if (msgheader->message_size != msgheader->total_header_size + msgheader->body_size + 0x20) return false;
	if (msgheader->message_size > MAX_MESSAGE_SIZE) return false; // prooobably too large

	if (msgheader->body_size <= 0x20) {
		u8 b = 0;
		u8* ptr = msgbuf + msgheader->total_header_size;
		for (int i = 0; i < msgheader->body_size; i++) {
			b |= *ptr;
			ptr++;
		}
		if (b == 0) return false;
	}

	return true;
}

Result addStreetpassMessage(u8* msgbuf) {
	Result res = 0;
	CecMessageHeader* msgheader = (CecMessageHeader*)msgbuf;
	// sanity checks
	if (!validateStreetpassMessage(msgbuf)) return -1; // bad message

	// update the msg header about the receive time
	if (!msgheader->received.year) {
		getCurrentTime(&(msgheader->received));
	}

	// first fetch how large the boxbuf is
	u8* boxbuf = malloc(sizeof(CecBoxInfoHeader));
	if (!boxbuf) {
		return -3;
	}
	res = cecdOpenAndRead(msgheader->title_id, CEC_PATH_INBOX_INFO, sizeof(CecBoxInfoHeader), boxbuf);
	if (R_FAILED(res)) {
		res = -2; // cecd fild not found
		goto cleanup_box;
	}
	// let's open the box buffer to see if we can even accept a new message, or if the message has already been accepted
	int max_boxbuf_size = sizeof(CecBoxInfoHeader) + sizeof(CecMessageHeader) * ((CecBoxInfoHeader*)boxbuf)->max_num_messages;
	free(boxbuf);
	boxbuf = malloc(max_boxbuf_size);
	if (!boxbuf) {
		return -3;
	}
	res = cecdOpenAndRead(msgheader->title_id, CEC_PATH_INBOX_INFO, max_boxbuf_size, boxbuf);
	if (R_FAILED(res)) {
		res = -2; // cecd file not found
		goto cleanup_box;
	}
	CecBoxInfoHeader* boxheader = (CecBoxInfoHeader*)boxbuf;
	CecMessageHeader* boxmsgs = (CecMessageHeader*)(boxbuf + sizeof(CecBoxInfoHeader));

	for (int i = 0; i < boxheader->num_messages; i++) {
		if (0 == memcmp(boxmsgs[i].message_id, msgheader->message_id, sizeof(CecMessageId))){
			res = -4; // already added, nothing to do
			goto cleanup_box;
		}
	}
	if (boxheader->num_messages >= boxheader->max_num_messages) {
		res = -5; // box already full
		goto cleanup_box;
	}

	// let's see if the message is too large for this box
	if (boxheader->max_message_size < msgheader->message_size) {
		res = -1;
		goto cleanup_box;
	}

	// now let's check if the box would overflow
	if (boxheader->box_size + msgheader->message_size > boxheader->max_box_size) {
		res = -1;
		goto cleanup_box;
	}

	// cool, the checks passed. Let's free the buffer for now, add our message, and then check if we gotta update the box
	free(boxbuf);

	{
		// box stuffs is done, let's fetch the mbox, to fetch the hmac key
		CecMBoxInfoHeader mboxheader;
		res = cecdOpenAndRead(msgheader->title_id, CEC_PATH_MBOX_INFO, sizeof(CecMBoxInfoHeader), (u8*)&mboxheader);
		if (R_FAILED(res)) return -2;

		msgheader->unopened = true;
		msgheader->new_flag = true;
		// great,we have all the bits we need now
		res = cecdWriteMessageWithHMAC(
			msgheader->title_id, false,
			msgheader->message_size, msgbuf,
			msgheader->message_id, mboxheader.hmac_key);
		if (R_FAILED(res)) return res;

		// check if the message was actually added...
		res = cecdReadMessage(msgheader->title_id, false, 0, 0, msgheader->message_id);
		if (R_FAILED(res)) return res;
	}

	// let's see if we gotta update the metadata
	boxbuf = malloc(max_boxbuf_size);
	if (!boxbuf) {
		return -3;
	}
	res = cecdOpenAndRead(msgheader->title_id, CEC_PATH_INBOX_INFO, max_boxbuf_size, boxbuf);
	if (R_FAILED(res)) {
		res = -2; // cecd fild not found
		goto cleanup_box;
	}

	boxheader = (CecBoxInfoHeader*)boxbuf;
	boxmsgs = (CecMessageHeader*)(boxbuf + sizeof(CecBoxInfoHeader));
	bool found_box = false;
	for (int i = 0; i < boxheader->num_messages; i++) {
		if (0 == memcmp(boxmsgs[i].message_id, msgheader->message_id, sizeof(CecMessageId))){
			found_box = true;
			break;
		}
	}
	
	if (!found_box) {
		// ok, let's add the message to the box
		memcpy(boxbuf + sizeof(CecBoxInfoHeader) + sizeof(CecMessageHeader) * ((CecBoxInfoHeader*)boxbuf)->num_messages, msgbuf, sizeof(CecMessageHeader));
		boxheader->num_messages++;
		boxheader->file_size += sizeof(CecMessageHeader);
		boxheader->box_size += msgheader->message_size;
		res = cecdOpenAndWrite(msgheader->title_id, CEC_PATH_INBOX_INFO, boxheader->file_size, boxbuf);
		if (R_FAILED(res)) {
			res = -2; // cecd fild not found
			goto cleanup_box;
		}
		free(boxbuf);
	}


	{
		// now set the green notification dot
		// gotta re-fetch the mbox header, in case it changed
		CecMBoxInfoHeader mboxheader;
		res = cecdOpenAndRead(msgheader->title_id, CEC_PATH_MBOX_INFO, sizeof(CecMBoxInfoHeader), (u8*)&mboxheader);
		if (R_FAILED(res)) return -2;

		getCurrentTime(&(mboxheader.last_received));
		mboxheader.flag3 = 1; // set the new notification dot
		//mboxheader.flag4 = 1;
		//mboxheader.flag5 = 1;
		//mboxheader.flag6 = 1;
		res = cecdOpenAndWrite(msgheader->title_id, CEC_PATH_MBOX_INFO, sizeof(CecMBoxInfoHeader), (u8*)&mboxheader);
		if (R_FAILED(res)) return -2;
	}

	return res;
cleanup_box:
	free(boxbuf);
	return res;
}