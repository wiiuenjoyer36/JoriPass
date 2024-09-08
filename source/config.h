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

typedef struct {
	int last_location;
	int language;
	u16 year;
	u8 month;
	u8 day;
	u32 price;
	u32 title_ids_ignored[24];
} Config;

void addIgnoredTitle(u32 title_id);
void removeIgnoredTitle(u32 title_id);
bool isTitleIgnored(u32 title_id);

void configInit(void);
void configWrite(void);

extern Config config;