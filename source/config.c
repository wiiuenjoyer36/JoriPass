/**
 * NetPass
 * Copyright (C) 2024 Sorunome, SunOfLife1
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

#include "config.h"
#include "strings.h"
#include "utils.h"
#include "cecd.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char config_path[] = "/config/netpass/netpass.cfg";

Config config = {
	.last_location = -1,
	.language = -1,
	.year = 0,
	.month = 0,
	.day = 0,
	.price = 0,
};

void addIgnoredTitle(u32 title_id) {
	for (size_t i = 0; i < 24; i++) {
		// If title_id is already ignored, just return
		if (config.title_ids_ignored[i] == title_id) return;

		// Add title_id to first empty spot in array
		if (config.title_ids_ignored[i] == 0) {
			config.title_ids_ignored[i] = title_id;
			return;
		}
	}
}

void removeIgnoredTitle(u32 title_id) {
	for (size_t i = 0; i < 24; i++) {
		if (config.title_ids_ignored[i] == title_id) {
			config.title_ids_ignored[i] = 0;
			return;
		}
	}
}

bool isTitleIgnored(u32 title_id) {
	for (size_t i = 0; i < 24; i++) {
		if (config.title_ids_ignored[i] == title_id) return true;
	}
	return false;
}

void load(void) {
	FILE* f = fopen(config_path, "r");
	if (!f) return;
	char line[200];
	while (fgets(line, 200, f)) {
		char* separator = strchr(line, '=');
		if (!separator) continue;
		char* key = line;
		*separator = '\0';
		char* value = separator + 1;
		char* eol = strchr(value, '\n');
		if (eol) {
			*eol = '\0';
		}
		if (!key[0] || !value[0]) continue;
		int key_len = strlen(key);
		int value_len = strlen(value);
		for (int i = 0; i < key_len; i++) key[i] = toupper(key[i]);
		for (int i = 0; i < value_len; i++) value[i] = toupper(value[i]);
		if (strcmp(key, "LANGUAGE") == 0) {
			if (strcmp(value, "SYSTEM") == 0) {
				config.language = -1;
			} else {
				for (int i = 0; i < NUM_LANGUAGES; i++) {
					if (strcmp(value, all_languages_str[i]) == 0) {
						config.language = all_languages[i];
						break;
					}
				}
			}
		}
		if (strcmp(key, "LAST_LOCATION") == 0) {
			config.last_location = atoi(value);
		}
		if (strcmp(key, "YEAR") == 0) {
			config.year = atoi(value);
		}
		if (strcmp(key, "MONTH") == 0) {
			config.month = atoi(value);
		}
		if (strcmp(key, "DAY") == 0) {
			config.day = atoi(value);
		}
		if (strcmp(key, "PRICE") == 0) {
			config.price = atoi(value);
		}
		if (strcmp(key, "TITLE_IDS_IGNORED") == 0) {
			// Open mbox_list now to avoid repeatedly doing it later
			Result res = 0;
			CecMboxListHeader mbox_list;
			res = cecdOpenAndRead(0, CEC_PATH_MBOX_LIST, sizeof(CecMboxListHeader), (u8*)&mbox_list);
			if (R_FAILED(res)) continue;
			
			// Read titles ids
			for (size_t i = 0; i < 24; i++) {
				sscanf(&value[9*i], "%lx,", &config.title_ids_ignored[i]);
				
				// Remove title id if not in mbox_list
				if (config.title_ids_ignored[i] == 0) continue;
				bool found = false;
				for (size_t j = 0; j < mbox_list.num_boxes; j++) {
					u32 title_id = strtol((const char*)mbox_list.box_names[j], NULL, 16);
					if (title_id == config.title_ids_ignored[i]) {
						found = true;
						break;
					}
				}
				if (!found) config.title_ids_ignored[i] = 0;
			}
		}
	}
	fclose(f);
}

void configWrite(void) {
	FILE* f = fopen(config_path, "w");
	char line[250];
	snprintf(line, 250, "last_location=%d\n", config.last_location);
	fputs(line, f);
	snprintf(line, 250, "year=%d\n", config.year);
	fputs(line, f);
	snprintf(line, 250, "month=%d\n", config.month);
	fputs(line, f);
	snprintf(line, 250, "day=%d\n", config.day);
	fputs(line, f);
	snprintf(line, 250, "price=%ld\n", config.price);
	fputs(line, f);
	if (config.language == -1) {
		fputs("language=system\n", f);
	} else {
		for (int i = 0; i < NUM_LANGUAGES; i++) {
			if (config.language == all_languages[i]) {
				snprintf(line, 250, "language=%s\n", all_languages_str[i]);
				fputs(line, f);
				break;
			}
		}
	}
	snprintf(line, 250, "title_ids_ignored=");
	for (size_t i = 0; i < 24; i++) {
		snprintf(line + 18 + (9*i), 250 - (18 + (9*i)), "%08lx,", config.title_ids_ignored[i]);
	}
	snprintf(line + 18 + (9*24), 250 - (18 + (9*24)), "\n");
	fputs(line, f);
	
	fclose(f);
}

void configInit(void) {
	mkdir_p((char*)config_path);
	load();
}