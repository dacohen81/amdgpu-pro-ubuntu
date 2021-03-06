#ifndef AMDGPU_BACKPORT_H
#define AMDGPU_BACKPORT_H

#include <kcl/kcl_drm.h>
#include <kcl/kcl_ttm.h>
#include <kcl/kcl_amdgpu.h>
#include <kcl/kcl_fence.h>
#include <kcl/kcl_vga_switcheroo.h>
#include <kcl/kcl_mm.h>
#include <kcl/kcl_fence_array.h>
#include <kcl/kcl_fs.h>
#include <kcl/kcl_acpi.h>
#include <kcl/kcl_hwmon.h>
#include <kcl/kcl_device.h>
#include <kcl/kcl_tracepoint.h>
#include <kcl/kcl_mn.h>
#include <kcl/kcl_fence_array.h>
#include <kcl/kcl_kthread.h>

void amdgpu_kcl_init(void);
#include <kcl/kcl_fence_array.h>

#endif /* AMDGPU_BACKPORT_H */
