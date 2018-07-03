/*
 * Stub module loader
 * Fixes Wi-Fi using built-in pronto_wlan on stock ROM
 * 
 * Copyright (C) 2018 Khronodragon <kdrag0n@protonmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version
 * 2 of the Licence, or (at your option) any later version.
 */

#include <linux/syscalls.h>

SYSCALL_DEFINE3(finit_module, int, fd, const char __user *, uargs, int, flags)
{
	pr_info("finit_module: faking success for fd=%d, uargs=%p, flags=%i\n", fd, uargs, flags);
	return 0;
}
