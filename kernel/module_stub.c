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

static void *m_start(struct seq_file *m, loff_t *pos)
{
	mutex_lock(&module_mutex);
	return seq_list_start(&modules, *pos);
}

static void *m_next(struct seq_file *m, void *p, loff_t *pos)
{
	return seq_list_next(p, &modules, pos);
}

static void m_stop(struct seq_file *m, void *p)
{
	mutex_unlock(&module_mutex);
}

static int m_show(struct seq_file *m, void *p)
{
	struct module *mod = list_entry(p, struct module, list);
	char buf[8];

	/* We always ignore unformed modules. */
	if (mod->state == MODULE_STATE_UNFORMED)
		return 0;

	seq_printf(m, "%s %u",
			   mod->name, mod->init_size + mod->core_size);
	print_unload_info(m, mod);

	/* Informative for users. */
	seq_printf(m, " %s",
			   mod->state == MODULE_STATE_GOING ? "Unloading" : mod->state == MODULE_STATE_COMING ? "Loading" : "Live");
	/* Used by oprofile and other similar tools. */
	seq_printf(m, " 0x%pK", mod->module_core);

	/* Taints info */
	if (mod->taints)
		seq_printf(m, " %s", module_flags(mod, buf));

	seq_printf(m, "\n");
	return 0;
}

/* Format: modulename size refcount deps address

   Where refcount is a number or -, and deps is a comma-separated list
   of depends or -.
*/
static const struct seq_operations modules_op = {
	.start = m_start,
	.next = m_next,
	.stop = m_stop,
	.show = m_show};

static int modules_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &modules_op);
}

static const struct file_operations proc_modules_operations = {
	.open = modules_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int __init proc_modules_init(void)
{
	proc_create("modules", 0, NULL, &proc_modules_operations);
	return 0;
}
module_init(proc_modules_init);
