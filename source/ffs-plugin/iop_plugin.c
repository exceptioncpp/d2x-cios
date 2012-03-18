/*
 * FFS plugin for Custom IOS.
 *
 * Copyright (C) 2009-2010 Waninkoko.
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

#include <string.h>

#include "direct_syscalls.h"
#include "iop_calls.h"
#include "isfs.h"
#include "fat.h"
#include "fs_tools.h"
#include "plugin.h"
#include "stealth.h"
#include "swi_mload.h"
#include "syscalls.h"
#include "tools.h"
#include "types.h"

//#define IOP_TRACE_OPEN

static char* device_null = "/dev/null";

/* Function pointer */
static TRCheckFunc CheckThreadRights = NULL;

static void __IOP_LogUnauthorizedRequest(char *path)
{
	svc_write("IOP: Unauthorized access to '");
	svc_write(path);
	svc_write("'. Blocking opening request.\n");
}

static s32 __IOP_CheckOpeningRequest(char *path, u32 rights)
{
	s32 tid, ret;

	/* Lazily set thread rights check function */
	if (CheckThreadRights == NULL)
		CheckThreadRights = Swi_GetThreadRightsCheckFunc();

	/* Get current thread id */
	tid = direct_os_get_thread_id();

	/* Check thread rights */
	ret = CheckThreadRights(tid, rights);

	/* Log unauthorized request */
	if (!ret)
		__IOP_LogUnauthorizedRequest(path);

	return ret;
}

char *__IOP_SyscallOpen(char *path, s32 mode)
{
	static char newpath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);

	s32 ret;

#ifdef IOP_TRACE_OPEN
	svc_write("IOP: open ");svc_write(path);svc_write("\n");
#endif

	/* Custom path */
	if (*path != '/') {
		/*
		 * Paths starting with '#' are always sent to real nand.
		 * This is an internal feature, only authorized threads 
		 * can use it.
		 */
		if (*path == '#') {
			/* Check opening request */
			ret = __IOP_CheckOpeningRequest(path, TID_RIGHTS_FORCE_REAL_NAND);
	
			/* Block request returning original invalid path */
			if (!ret)
				return path;

			/* Copy path */
			strcpy(newpath, path);

			/* Replace first character */
			*newpath = '/';

			/* Set flag to force real path */
			forceRealPath = 1;

			/* Return new path */
			return newpath;
		}

		/* Emulate old fat module to keep backward compatibility */
		if (!strcmp(path, "fat")) {
			/* Illegal when a title is running */
			if (Swi_GetRunningTitle()) {
				__IOP_LogUnauthorizedRequest(path);

				/* Replace path */
				strcpy(newpath, device_null);
			}
			else {
				svc_write("IOP: Rerouting FAT to /dev/fs for backward compatibility.\n");

				/* Replace path */
				strcpy(newpath, "/dev/fs");
			}

			/* Return new path */
			return newpath;
		}
	}

	/* Emulation mode */
	if (config.mode) {

		/* SDHC mode */
		if (FS_GetDevice(config.mode) == FS_MODE_SDHC) {
			if (!strcmp(path, "/dev/sdio/slot0")) {

				/* Log */
				svc_write("IOP: The game is trying to open /dev/sdio/slot0.\n");
				svc_write("IOP: To avoid interferences with emu nand on sd card the request is redirected to /dev/null.\n");

				/* Replace path */
				strcpy(newpath, device_null);

				/* Return new path */
				return newpath;
			}
		}
	}

	/* Return original path */
	return path;
}

