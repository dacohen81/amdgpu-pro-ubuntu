#ifndef AMDGPU_BACKPORT_KCL_ACPI_H
#define AMDGPU_BACKPORT_KCL_ACPI_H

/**
 * interface change in mainline kernel 3.13
 * but only affect RHEL6 without backport
 */
#if (defined OS_NAME_RHEL) && (OS_VERSION_MAJOR <= 6)

#include <acpi/acpi_bus.h>

#define ACPI_HANDLE(dev) DEVICE_ACPI_HANDLE(dev)

#endif

#endif /* AMDGPU_BACKPORT_KCL_ACPI_H */
