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

#include "utils.h"
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>

// from https://nachtimwald.com/2017/11/18/base64-encode-and-decode-in-c/
size_t b64_encoded_size(size_t inlen) {
	size_t ret;

	ret = inlen;
	if (inlen % 3 != 0)
		ret += 3 - (inlen % 3);
	ret /= 3;
	ret *= 4;

	return ret;
}

char* b64encode(u8* in, size_t len) {
	const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";

	if (in == NULL || len == 0) return NULL;

	size_t elen = b64_encoded_size(len);
	char* out = malloc(elen + 1);
	out[elen] = '\0';

	for (size_t i = 0, j = 0; i < len; i += 3, j += 4) {
		size_t v = in[i];
		v = in[i];
		v = i+1 < len ? v << 8 | in[i+1] : v << 8;
		v = i+2 < len ? v << 8 | in[i+2] : v << 8;

		out[j]   = b64chars[(v >> 18) & 0x3F];
		out[j+1] = b64chars[(v >> 12) & 0x3F];
		if (i+1 < len) {
			out[j+2] = b64chars[(v >> 6) & 0x3F];
		} else {
			out[j+2] = '\0';
		}
		if (i+2 < len) {
			out[j+3] = b64chars[v & 0x3F];
		} else {
			out[j+3] = '\0';
		}
	}
	return out;
}



// from https://stackoverflow.com/a/2256974
int rmdir_r(char *path) {
	DIR *d = opendir(path);
	size_t path_len = strlen(path);
	int r = -1;

	if (d) {
		struct dirent *p;

		r = 0;
		while (!r && (p=readdir(d))) {
			int r2 = -1;
			char *buf;
			size_t len;

			/* Skip the names "." and ".." as we don't want to recurse on them. */
			if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
				continue;

			len = path_len + strlen(p->d_name) + 2; 
			buf = malloc(len);

			if (buf) {
				struct stat statbuf;

				snprintf(buf, len, "%s/%s", path, p->d_name);
				if (!stat(buf, &statbuf)) {
					if (S_ISDIR(statbuf.st_mode))
						r2 = rmdir_r(buf);
					else
						r2 = unlink(buf);
				}
				free(buf);
			}
			r = r2;
		}
		closedir(d);
	}

	if (!r)
		r = rmdir(path);

	return r;
}

void mkdir_p(char* orig_path) {
	int maxlen = strlen(orig_path) + 1;
	char path[maxlen];
	memcpy(path, orig_path, maxlen);
	path[maxlen - 1] = 0;
	int pos = 0;
	do {
		char* found = strchr(path + pos + 1, '/');
		if (!found) {
			break;
		}
		*found = '\0';
		mkdir(path, 777);
		*found = '/';
		pos = (int)found - (int)path;
	} while(pos < maxlen);
}

Result APT_Wrap(u32 in_size, void* in, u32 nonce_offset, u32 nonce_size, u32 out_size, void* out) {
	u32 cmdbuf[16];
	cmdbuf[0] = IPC_MakeHeader(0x46, 4, 4); // 0x001F0084
	cmdbuf[1] = out_size;
	cmdbuf[2] = in_size;
	cmdbuf[3] = nonce_offset;
	cmdbuf[4] = nonce_size;

	cmdbuf[5] = IPC_Desc_Buffer(in_size, IPC_BUFFER_R);
	cmdbuf[6] = (u32)in;
	cmdbuf[7] = IPC_Desc_Buffer(out_size, IPC_BUFFER_W);
	cmdbuf[8] = (u32)out;

	Result res = aptSendCommand(cmdbuf);
	if (R_FAILED(res)) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result APT_Unwrap(u32 in_size, void* in, u32 nonce_offset, u32 nonce_size, u32 out_size, void* out) {
	u32 cmdbuf[16];
	cmdbuf[0] = IPC_MakeHeader(0x47, 4, 4); // 0x001F0084
	cmdbuf[1] = out_size;
	cmdbuf[2] = in_size;
	cmdbuf[3] = nonce_offset;
	cmdbuf[4] = nonce_size;

	cmdbuf[5] = IPC_Desc_Buffer(in_size, IPC_BUFFER_R);
	cmdbuf[6] = (u32)in;
	cmdbuf[7] = IPC_Desc_Buffer(out_size, IPC_BUFFER_W);
	cmdbuf[8] = (u32)out;

	Result res = aptSendCommand(cmdbuf);
	if (R_FAILED(res)) return res;
	res = (Result)cmdbuf[1];

	return res;
}

// From libctru https://github.com/devkitPro/libctru/blob/faf5162b60eab5402d3839330f985b84382df76c/libctru/source/applets/miiselector.c#L153
u16 crc16_ccitt(void const *buf, size_t len, uint32_t starting_val) {
	if (!buf)
		return -1;

	u8 const *cbuf = buf;
	u32 crc        = starting_val;

	static const u16 POLY = 0x1021;

	for (size_t i = 0; i < len; i++)
	{
		for (int bit = 7; bit >= 0; bit--)
			crc = ((crc << 1) | ((cbuf[i] >> bit) & 0x1)) ^ (crc & 0x8000 ? POLY : 0);
	}

	for (int _ = 0; _ < 16; _++)
		crc = (crc << 1) ^ (crc & 0x8000 ? POLY : 0);

	return (u16)(crc & 0xffff);
}

Result decryptMii(void* data, MiiData* mii) {
	MiiData* out = malloc(sizeof(MiiData) + 4);
	Result res = APT_Unwrap(0x70, data, 12, 10, sizeof(MiiData) + 4, out);
	if (R_FAILED(res)) goto error;
	if (out->magic != 0x03) {
		res = -1;
		goto error;
	}

	u16 crc_calc = crc16_ccitt(out, sizeof(MiiData) + 2, 0);
	u16 crc_check = __builtin_bswap16(*(u16*)(((u8*)out) + sizeof(MiiData) + 2));
	if (crc_calc != crc_check) {
		res = -1;
		goto error;
	}

	memcpy(mii, out, sizeof(MiiData));

error:
	free(out);
	return res;
}

u8* memsearch(u8* buf, size_t buf_len, u8* cmp, size_t cmp_len) {
	u8* buf_orig = buf;
	while (buf_len - ((int)(buf - buf_orig)) > 0 && (buf = memchr(buf, *(uint8_t*)cmp, buf_len - ((int)(buf - buf_orig))))) {
		if (memcmp(buf, cmp, cmp_len) == 0) {
			return buf;
		}
		buf++;
	}
	return NULL;
}