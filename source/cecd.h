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

#define CEC_PATH_MBOX_LIST 1
#define CEC_PATH_MBOX_INFO 2
#define CEC_PATH_INBOX_INFO 3
#define CEC_PATH_OUTBOX_INFO 4
#define CEC_PATH_OUTBOX_INDEX 5
#define CEC_PATH_INBOX_MSG 6
#define CEC_PATH_OUTBOX_MSG 7
#define CEC_PATH_ROOT_DIR 10
#define CEC_PATH_MBOX_DIR 11
#define CEC_PATH_INBOX_DIR 12
#define CEC_PATH_OUTBOX_DIR 13
#define CECMESSAGE_BOX_ICON 101
#define CECMESSAGE_BOX_TITLE 110

#define CEC_COMMAND_NONE 0
#define CEC_COMMAND_START 1
#define CEC_COMMAND_RESET_START 2
#define CEC_COMMAND_READYSCAN 3
#define CEC_COMMAND_READYSCANWAIT 4
#define CEC_COMMAND_STARTSCAN 5
#define CEC_COMMAND_RESCAN 6
#define CEC_COMMAND_NDM_RESUME 7
#define CEC_COMMAND_NDM_SUSPEND 8
#define CEC_COMMAND_NDM_SUSPEND_IMMEDIATE 9
#define CEC_COMMAND_STOPWAIT 0xA
#define CEC_COMMAND_STOP 0xB
#define CEC_COMMAND_STOP_FORCE 0xC
#define CEC_COMMAND_STOP_FORCE_WAIT 0xD
#define CEC_COMMAND_RESET_FILTER 0xE
#define CEC_COMMAND_DAEMON_STOP 0xF
#define CEC_COMMAND_DAEMON_START 0x10
#define CEC_COMMAND_EXIT 0x11
#define CEC_COMMAND_OVER_BOSS 0x12
#define CEC_COMMAND_OVER_BOSS_FORCE 0x13
#define CEC_COMMAND_OVER_BOSS_FORCE_WAIT 0x14
#define CEC_COMMAND_END 0x15

#define CEC_STATE_ABBREV_IDLE 1
#define CEC_STATE_ABBREV_INACTTIVE 2
#define CEC_STATE_ABBREV_SCANNING 3
#define CEC_STATE_ABBREV_WLREADY 4
#define CEC_STATE_ABBREV_OTHER 5

// only a made-up number that is prolly large enough for existing streetpass messages
#define MAX_MESSAGE_SIZE 0x20000

typedef u8 CecMessageId[8];

Result cecdInit(void);
Result cecdGetState(u32* state);
Result cecdGetSystemInfo(u32 destbuf_size, void* destbuf);
Result cecdReadMessage(u32 program_id, bool is_outbox, u32 size, u8* buf, CecMessageId message_id);
Result cecdReadMessageWithHMAC(u32 program_id, bool is_outbox, u32 size, u8* buf, CecMessageId message_id, u8* hmac);
Result cecdWriteMessage(u32 program_id, bool is_outbox, u32 size, u8* buf, CecMessageId message_id);
Result cecdWriteMessageWithHMAC(u32 program_id, bool is_outbox, u32 size, u8* buf, CecMessageId message_id, u8* hmac);
Result cecdGetCecInfoEventHandle(Handle* handle);
Result cecdGetChangeStateEventHandle(Handle* handle);
Result cecdOpenAndWrite(u32 program_id, u32 path_type, u32 size, u8* buf);
Result cecdOpenAndRead(u32 program_id, u32 path_type, u32 size, u8* buf);
Handle cecdGetServHandle(void);

Result updateStreetpassOutbox(u8* msgbuf);
bool validateStreetpassMessage(u8* msgbuf);
Result addStreetpassMessage(u8* msgbuf);

typedef struct {
	u32 magic; // 0x42504643 CFPB
	u8 unknown[0x14];
	union {
		u8 nonce[0x8];
		struct{
			u32 mii_id;
			u8 top_mac[4];
		};
	};
	u8 ciphertext[0x58];
	u8 mac[0x10];
} CFPB;

typedef struct CecTimestamp {
	u32 year;
	u8 month;
	u8 day;
	u8 weekday;
	u8 hour;
	u8 minute;
	u8 second;
	u16 millisecond;
} CecTimestamp;

typedef struct CecMessageHeader {
	u16 magic; // 0x6060 ``
	u16 padding;
	u32 message_size;
	u32 total_header_size;
	u32 body_size;
	u32 title_id;
	u32 title_id2;
	u32 batch_id;
	u32 unknown_1;
	CecMessageId message_id;
	u32 message_version;
	CecMessageId message_id2;
	u8 flags;
	u8 send_method;
	bool unopened;
	bool new_flag;
	u64 sender_id;
	u64 sender_id2;
	CecTimestamp sent;
	CecTimestamp received;
	CecTimestamp created;
	u8 send_count;
	u8 forward_count;
	u16 user_data;
} CecMessageHeader;

typedef struct CecBoxInfoHeader {
	u16 magic; // 0x6262 'bb'
	u16 padding;
	u32 file_size;
	u32 max_box_size;
	u32 box_size;
	u32 max_num_messages;
	u32 num_messages;
	u32 max_batch_size;
	u32 max_message_size;
} CecBoxInfoHeader;

typedef struct CecBoxInfoFull {
	CecBoxInfoHeader header;
	CecMessageHeader messages[10];
} CecBoxInfoFull;

typedef struct CecMBoxInfoHeader {
	u16 magic; // 0x6363 'cc'
	u16 padding1;
	u32 program_id;
	u32 private_id;
	u8 flag;
	u8 flag2;
	u16 padding2;
	u8 hmac_key[32];
	u32 padding3;
	CecTimestamp last_accessed;
	u8 flag3;
	u8 flag4;
	u8 flag5;
	u8 flag6;
	CecTimestamp last_received;
	u32 padding5;
	CecTimestamp unknown_time;
} CecMBoxInfoHeader;

typedef struct CecMboxListHeader {
	u16 magic; // 0x6868 'hh'
	u16 padding;
	u32 version;
	u32 num_boxes;
	u8 box_names[24][16]; // 12 used, but space for 24
} CecMboxListHeader;