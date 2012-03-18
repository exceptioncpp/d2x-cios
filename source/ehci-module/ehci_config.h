/*
 * ES plugin for Custom IOS.
 *
 * Copyright (C) 2010 Waninkoko.
 * Copyright (C) 2011 davebaol.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EHCI_CONFIG_H_
#define _EHCI_CONFIG_H_

#include "types.h"

#define USECS_PER_SEC   (1000 * 1000)

typedef struct {
	u32 useUsbPort1;
	u32 watchdogTimeout;
} ehciConfig;

extern ehciConfig config;

/* Prototypes */
s32 EHCI_LoadConfig(void);
s32 EHCI_SaveConfig(void);

#endif

