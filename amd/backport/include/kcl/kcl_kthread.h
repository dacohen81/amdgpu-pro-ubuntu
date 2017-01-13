#ifndef AMDGPU_BACKPORT_KCL_KTHREAD_H
#define AMDGPU_BACKPORT_KCL_KTHREAD_H

#include <linux/sched.h>
#include <linux/kthread.h>

extern void (*_kcl_kthread_parkme)(void);
extern void (*_kcl_kthread_unpark)(struct task_struct *k);
extern int (*_kcl_kthread_park)(struct task_struct *k);
extern bool (*_kcl_kthread_should_park)(void);

static inline void kcl_kthread_parkme(void)
{
	return _kcl_kthread_parkme();
}

static inline void kcl_kthread_unpark(struct task_struct *k)
{
	return _kcl_kthread_unpark(k);
}

static inline int kcl_kthread_park(struct task_struct *k)
{
	return _kcl_kthread_park(k);
}

static inline bool kcl_kthread_should_park(void)
{
	return _kcl_kthread_should_park();
}

#endif /* AMDGPU_BACKPORT_KCL_KTHREAD_H */
