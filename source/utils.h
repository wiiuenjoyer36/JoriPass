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
#include <3ds/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define MAX(x, y) (x > y ? x : y)

char* b64encode(u8* in, size_t len);
int rmdir_r(char *path);
void mkdir_p(char* orig_path);
Result APT_Wrap(u32 in_size, void* in, u32 nonce_offset, u32 nonce_size, u32 out_size, void* out);
Result APT_Unwrap(u32 in_size, void* in, u32 nonce_offset, u32 nonce_size, u32 out_size, void* out);
u16 crc16_ccitt(void const *buf, size_t len, uint32_t starting_val);
Result decryptMii(void* data, MiiData* mii);
u8* memsearch(u8* buf, size_t buf_len, u8* cmp, size_t cmp_len);

typedef struct {
	u32 magic; // 0x4F00
	u16 total_coins;
	u16 today_coins;
	u32 total_step_count_last_coin;
	u32 today_step_count_last_coin;
	u16 year;
	u8 month;
	u8 day;
} PlayCoins;
