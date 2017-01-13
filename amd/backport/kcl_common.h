#ifndef AMDGPU_BACKPORT_KCL_COMMON_H
#define AMDGPU_BACKPORT_KCL_COMMON_H

/*
 * kallsyms_lookup_name has been exported in version 2.6.33
 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
#include <linux/kallsyms.h>
#else
extern
unsigned long (*kallsyms_lookup_name)(const char *name);
#endif

static inline void *amdgpu_kcl_fp_setup(const char *symbol, void *fp_stup)
{
	unsigned long addr;
	void *fp = NULL;

	addr = kallsyms_lookup_name(symbol);
	if (addr == 0) {
		fp = fp_stup;
		if (fp != NULL)
			printk_once(KERN_WARNING "Warning: fail to get symbol %s, replace it with kcl stub\n", symbol);
		else {
			printk_once(KERN_ERR "Error: fail to get symbol %s\n", symbol);
			BUG();
		}
	} else {
		fp = (void *)addr;
	}

	return fp;
}
#endif
