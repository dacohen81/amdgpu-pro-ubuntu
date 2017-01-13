
extern void kcl_ttm_init(void);
extern void kcl_kthread_init(void);
extern void kcl_drm_init(void);

void amdgpu_kcl_init(void)
{
	kcl_ttm_init();
	kcl_kthread_init();
	kcl_drm_init();
}
