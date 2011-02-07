#include <config.h>

#ifdef WITH_QEMU

# include <stdio.h>
# include <stdlib.h>

# include "testutils.h"
# include "qemu/qemu_capabilities.h"
# include "memory.h"

# define MAX_HELP_OUTPUT_SIZE 1024*64

struct testInfo {
    const char *name;
    unsigned long long flags;
    unsigned int version;
    unsigned int is_kvm;
    unsigned int kvm_version;
};

static char *progname;
static char *abs_srcdir;

static void printMismatchedFlags(unsigned long long got,
                                 unsigned long long expect)
{
    int i;

    for (i = 0 ; i < (sizeof(got)*CHAR_BIT) ; i++) {
        unsigned long long gotFlag = (got & (1LL << i));
        unsigned long long expectFlag = (expect & (1LL << i));
        if (gotFlag && !expectFlag)
            fprintf(stderr, "Extra flag %i\n", i);
        if (!gotFlag && expectFlag)
            fprintf(stderr, "Missing flag %i\n", i);
    }
}

static int testHelpStrParsing(const void *data)
{
    const struct testInfo *info = data;
    char *path = NULL;
    char helpStr[MAX_HELP_OUTPUT_SIZE];
    char *help = &(helpStr[0]);
    unsigned int version, is_kvm, kvm_version;
    unsigned long long flags;
    int ret = -1;

    if (virAsprintf(&path, "%s/qemuhelpdata/%s", abs_srcdir, info->name) < 0)
        return -1;

    if (virtTestLoadFile(path, &help, MAX_HELP_OUTPUT_SIZE) < 0)
        goto cleanup;

    if (qemuCapsParseHelpStr("QEMU", help, &flags,
                             &version, &is_kvm, &kvm_version) == -1)
        goto cleanup;

    if (info->flags & QEMU_CAPS_DEVICE) {
        VIR_FREE(path);
        if (virAsprintf(&path, "%s/qemuhelpdata/%s-device", abs_srcdir,
                        info->name) < 0)
            goto cleanup;

        if (virtTestLoadFile(path, &help, MAX_HELP_OUTPUT_SIZE) < 0)
            goto cleanup;

        if (qemuCapsParseDeviceStr(help, &flags) < 0)
            goto cleanup;
    }

    if (flags != info->flags) {
        fprintf(stderr,
                "Computed flags do not match: got 0x%llx, expected 0x%llx\n",
                flags, info->flags);

        if (getenv("VIR_TEST_DEBUG"))
            printMismatchedFlags(flags, info->flags);

        goto cleanup;
    }

    if (version != info->version) {
        fprintf(stderr, "Parsed versions do not match: got %u, expected %u\n",
                version, info->version);
        goto cleanup;
    }

    if (is_kvm != info->is_kvm) {
        fprintf(stderr,
                "Parsed is_kvm flag does not match: got %u, expected %u\n",
                is_kvm, info->is_kvm);
        goto cleanup;
    }

    if (kvm_version != info->kvm_version) {
        fprintf(stderr,
                "Parsed KVM versions do not match: got %u, expected %u\n",
                kvm_version, info->kvm_version);
        goto cleanup;
    }

    ret = 0;
cleanup:
    VIR_FREE(path);
    return ret;
}

static int
mymain(int argc, char **argv)
{
    int ret = 0;
    char cwd[PATH_MAX];

    progname = argv[0];

    if (argc > 1) {
        fprintf(stderr, "Usage: %s\n", progname);
        return (EXIT_FAILURE);
    }

    abs_srcdir = getenv("abs_srcdir");
    if (!abs_srcdir)
        abs_srcdir = getcwd(cwd, sizeof(cwd));

# define DO_TEST(name, flags, version, is_kvm, kvm_version)                          \
    do {                                                                            \
        const struct testInfo info = { name, flags, version, is_kvm, kvm_version }; \
        if (virtTestRun("QEMU Help String Parsing " name,                           \
                        1, testHelpStrParsing, &info) < 0)                          \
            ret = -1;                                                               \
    } while (0)

    DO_TEST("qemu-0.9.1",
            QEMU_CAPS_KQEMU |
            QEMU_CAPS_VNC_COLON |
            QEMU_CAPS_NO_REBOOT |
            QEMU_CAPS_DRIVE |
            QEMU_CAPS_NAME,
            9001,  0,  0);
    DO_TEST("kvm-74",
            QEMU_CAPS_VNC_COLON |
            QEMU_CAPS_NO_REBOOT |
            QEMU_CAPS_DRIVE |
            QEMU_CAPS_DRIVE_BOOT |
            QEMU_CAPS_NAME |
            QEMU_CAPS_VNET_HDR |
            QEMU_CAPS_MIGRATE_KVM_STDIO |
            QEMU_CAPS_KVM |
            QEMU_CAPS_DRIVE_FORMAT |
            QEMU_CAPS_MEM_PATH |
            QEMU_CAPS_TDF,
            9001,  1, 74);
    DO_TEST("kvm-83-rhel56",
            QEMU_CAPS_VNC_COLON |
            QEMU_CAPS_NO_REBOOT |
            QEMU_CAPS_DRIVE |
            QEMU_CAPS_DRIVE_BOOT |
            QEMU_CAPS_NAME |
            QEMU_CAPS_UUID |
            QEMU_CAPS_VNET_HDR |
            QEMU_CAPS_MIGRATE_QEMU_TCP |
            QEMU_CAPS_MIGRATE_QEMU_EXEC |
            QEMU_CAPS_DRIVE_CACHE_V2 |
            QEMU_CAPS_KVM |
            QEMU_CAPS_DRIVE_FORMAT |
            QEMU_CAPS_DRIVE_SERIAL |
            QEMU_CAPS_VGA |
            QEMU_CAPS_PCIDEVICE |
            QEMU_CAPS_MEM_PATH |
            QEMU_CAPS_BALLOON |
            QEMU_CAPS_RTC_TD_HACK |
            QEMU_CAPS_NO_HPET |
            QEMU_CAPS_NO_KVM_PIT |
            QEMU_CAPS_TDF |
            QEMU_CAPS_DRIVE_READONLY |
            QEMU_CAPS_SMBIOS_TYPE |
            QEMU_CAPS_SPICE,
            9001, 1,  83);
    DO_TEST("qemu-0.10.5",
            QEMU_CAPS_KQEMU |
            QEMU_CAPS_VNC_COLON |
            QEMU_CAPS_NO_REBOOT |
            QEMU_CAPS_DRIVE |
            QEMU_CAPS_NAME |
            QEMU_CAPS_UUID |
            QEMU_CAPS_MIGRATE_QEMU_TCP |
            QEMU_CAPS_MIGRATE_QEMU_EXEC |
            QEMU_CAPS_DRIVE_CACHE_V2 |
            QEMU_CAPS_DRIVE_FORMAT |
            QEMU_CAPS_DRIVE_SERIAL |
            QEMU_CAPS_VGA |
            QEMU_CAPS_0_10 |
            QEMU_CAPS_ENABLE_KVM |
            QEMU_CAPS_SDL |
            QEMU_CAPS_RTC_TD_HACK |
            QEMU_CAPS_NO_HPET |
            QEMU_CAPS_VGA_NONE,
            10005, 0,  0);
    DO_TEST("qemu-kvm-0.10.5",
            QEMU_CAPS_VNC_COLON |
            QEMU_CAPS_NO_REBOOT |
            QEMU_CAPS_DRIVE |
            QEMU_CAPS_DRIVE_BOOT |
            QEMU_CAPS_NAME |
            QEMU_CAPS_UUID |
            QEMU_CAPS_VNET_HDR |
            QEMU_CAPS_MIGRATE_QEMU_TCP |
            QEMU_CAPS_MIGRATE_QEMU_EXEC |
            QEMU_CAPS_DRIVE_CACHE_V2 |
            QEMU_CAPS_KVM |
            QEMU_CAPS_DRIVE_FORMAT |
            QEMU_CAPS_DRIVE_SERIAL |
            QEMU_CAPS_VGA |
            QEMU_CAPS_0_10 |
            QEMU_CAPS_PCIDEVICE |
            QEMU_CAPS_MEM_PATH |
            QEMU_CAPS_SDL |
            QEMU_CAPS_RTC_TD_HACK |
            QEMU_CAPS_NO_HPET |
            QEMU_CAPS_NO_KVM_PIT |
            QEMU_CAPS_TDF |
            QEMU_CAPS_NESTING |
            QEMU_CAPS_VGA_NONE,
            10005, 1,  0);
    DO_TEST("kvm-86",
            QEMU_CAPS_VNC_COLON |
            QEMU_CAPS_NO_REBOOT |
            QEMU_CAPS_DRIVE |
            QEMU_CAPS_DRIVE_BOOT |
            QEMU_CAPS_NAME |
            QEMU_CAPS_UUID |
            QEMU_CAPS_VNET_HDR |
            QEMU_CAPS_MIGRATE_QEMU_TCP |
            QEMU_CAPS_MIGRATE_QEMU_EXEC |
            QEMU_CAPS_DRIVE_CACHE_V2 |
            QEMU_CAPS_KVM |
            QEMU_CAPS_DRIVE_FORMAT |
            QEMU_CAPS_DRIVE_SERIAL |
            QEMU_CAPS_VGA |
            QEMU_CAPS_0_10 |
            QEMU_CAPS_PCIDEVICE |
            QEMU_CAPS_SDL |
            QEMU_CAPS_RTC_TD_HACK |
            QEMU_CAPS_NO_HPET |
            QEMU_CAPS_NO_KVM_PIT |
            QEMU_CAPS_TDF |
            QEMU_CAPS_NESTING |
            QEMU_CAPS_SMBIOS_TYPE |
            QEMU_CAPS_VGA_NONE,
            10050, 1,  0);
    DO_TEST("qemu-kvm-0.11.0-rc2",
            QEMU_CAPS_VNC_COLON |
            QEMU_CAPS_NO_REBOOT |
            QEMU_CAPS_DRIVE |
            QEMU_CAPS_DRIVE_BOOT |
            QEMU_CAPS_NAME |
            QEMU_CAPS_UUID |
            QEMU_CAPS_VNET_HDR |
            QEMU_CAPS_MIGRATE_QEMU_TCP |
            QEMU_CAPS_MIGRATE_QEMU_EXEC |
            QEMU_CAPS_DRIVE_CACHE_V2 |
            QEMU_CAPS_KVM |
            QEMU_CAPS_DRIVE_FORMAT |
            QEMU_CAPS_DRIVE_SERIAL |
            QEMU_CAPS_VGA |
            QEMU_CAPS_0_10 |
            QEMU_CAPS_PCIDEVICE |
            QEMU_CAPS_MEM_PATH |
            QEMU_CAPS_ENABLE_KVM |
            QEMU_CAPS_BALLOON |
            QEMU_CAPS_SDL |
            QEMU_CAPS_RTC_TD_HACK |
            QEMU_CAPS_NO_HPET |
            QEMU_CAPS_NO_KVM_PIT |
            QEMU_CAPS_TDF |
            QEMU_CAPS_BOOT_MENU |
            QEMU_CAPS_NESTING |
            QEMU_CAPS_NAME_PROCESS |
            QEMU_CAPS_SMBIOS_TYPE |
            QEMU_CAPS_VGA_NONE,
            10092, 1,  0);
    DO_TEST("qemu-0.12.1",
            QEMU_CAPS_VNC_COLON |
            QEMU_CAPS_NO_REBOOT |
            QEMU_CAPS_DRIVE |
            QEMU_CAPS_NAME |
            QEMU_CAPS_UUID |
            QEMU_CAPS_MIGRATE_QEMU_TCP |
            QEMU_CAPS_MIGRATE_QEMU_EXEC |
            QEMU_CAPS_DRIVE_CACHE_V2 |
            QEMU_CAPS_DRIVE_FORMAT |
            QEMU_CAPS_DRIVE_SERIAL |
            QEMU_CAPS_DRIVE_READONLY |
            QEMU_CAPS_VGA |
            QEMU_CAPS_0_10 |
            QEMU_CAPS_ENABLE_KVM |
            QEMU_CAPS_SDL |
            QEMU_CAPS_XEN_DOMID |
            QEMU_CAPS_MIGRATE_QEMU_UNIX |
            QEMU_CAPS_CHARDEV |
            QEMU_CAPS_BALLOON |
            QEMU_CAPS_DEVICE |
            QEMU_CAPS_SMP_TOPOLOGY |
            QEMU_CAPS_RTC |
            QEMU_CAPS_NO_HPET |
            QEMU_CAPS_BOOT_MENU |
            QEMU_CAPS_NAME_PROCESS |
            QEMU_CAPS_SMBIOS_TYPE |
            QEMU_CAPS_VGA_NONE |
            QEMU_CAPS_MIGRATE_QEMU_FD |
            QEMU_CAPS_DRIVE_AIO,
            12001, 0,  0);
    DO_TEST("qemu-kvm-0.12.1.2-rhel60",
            QEMU_CAPS_VNC_COLON |
            QEMU_CAPS_NO_REBOOT |
            QEMU_CAPS_DRIVE |
            QEMU_CAPS_DRIVE_BOOT |
            QEMU_CAPS_NAME |
            QEMU_CAPS_UUID |
            QEMU_CAPS_VNET_HDR |
            QEMU_CAPS_MIGRATE_QEMU_TCP |
            QEMU_CAPS_MIGRATE_QEMU_EXEC |
            QEMU_CAPS_DRIVE_CACHE_V2 |
            QEMU_CAPS_KVM |
            QEMU_CAPS_DRIVE_FORMAT |
            QEMU_CAPS_DRIVE_SERIAL |
            QEMU_CAPS_DRIVE_READONLY |
            QEMU_CAPS_VGA |
            QEMU_CAPS_0_10 |
            QEMU_CAPS_PCIDEVICE |
            QEMU_CAPS_MEM_PATH |
            QEMU_CAPS_MIGRATE_QEMU_UNIX |
            QEMU_CAPS_CHARDEV |
            QEMU_CAPS_ENABLE_KVM |
            QEMU_CAPS_BALLOON |
            QEMU_CAPS_DEVICE |
            QEMU_CAPS_SMP_TOPOLOGY |
            QEMU_CAPS_RTC |
            QEMU_CAPS_VNET_HOST |
            QEMU_CAPS_NO_KVM_PIT |
            QEMU_CAPS_TDF |
            QEMU_CAPS_PCI_CONFIGFD |
            QEMU_CAPS_NODEFCONFIG |
            QEMU_CAPS_BOOT_MENU |
            QEMU_CAPS_NESTING |
            QEMU_CAPS_NAME_PROCESS |
            QEMU_CAPS_SMBIOS_TYPE |
            QEMU_CAPS_VGA_QXL |
            QEMU_CAPS_SPICE |
            QEMU_CAPS_VGA_NONE |
            QEMU_CAPS_MIGRATE_QEMU_FD |
            QEMU_CAPS_DRIVE_AIO |
            QEMU_CAPS_DEVICE_SPICEVMC,
            12001, 1,  0);
    DO_TEST("qemu-kvm-0.12.3",
            QEMU_CAPS_VNC_COLON |
            QEMU_CAPS_NO_REBOOT |
            QEMU_CAPS_DRIVE |
            QEMU_CAPS_DRIVE_BOOT |
            QEMU_CAPS_NAME |
            QEMU_CAPS_UUID |
            QEMU_CAPS_VNET_HDR |
            QEMU_CAPS_MIGRATE_QEMU_TCP |
            QEMU_CAPS_MIGRATE_QEMU_EXEC |
            QEMU_CAPS_DRIVE_CACHE_V2 |
            QEMU_CAPS_KVM |
            QEMU_CAPS_DRIVE_FORMAT |
            QEMU_CAPS_DRIVE_SERIAL |
            QEMU_CAPS_DRIVE_READONLY |
            QEMU_CAPS_VGA |
            QEMU_CAPS_0_10 |
            QEMU_CAPS_PCIDEVICE |
            QEMU_CAPS_MEM_PATH |
            QEMU_CAPS_SDL |
            QEMU_CAPS_MIGRATE_QEMU_UNIX |
            QEMU_CAPS_CHARDEV |
            QEMU_CAPS_BALLOON |
            QEMU_CAPS_DEVICE |
            QEMU_CAPS_SMP_TOPOLOGY |
            QEMU_CAPS_RTC |
            QEMU_CAPS_VNET_HOST |
            QEMU_CAPS_NO_HPET |
            QEMU_CAPS_NO_KVM_PIT |
            QEMU_CAPS_TDF |
            QEMU_CAPS_BOOT_MENU |
            QEMU_CAPS_NESTING |
            QEMU_CAPS_NAME_PROCESS |
            QEMU_CAPS_SMBIOS_TYPE |
            QEMU_CAPS_VGA_NONE |
            QEMU_CAPS_MIGRATE_QEMU_FD |
            QEMU_CAPS_DRIVE_AIO,
            12003, 1,  0);
    DO_TEST("qemu-kvm-0.13.0",
            QEMU_CAPS_VNC_COLON |
            QEMU_CAPS_NO_REBOOT |
            QEMU_CAPS_DRIVE |
            QEMU_CAPS_DRIVE_BOOT |
            QEMU_CAPS_NAME |
            QEMU_CAPS_UUID |
            QEMU_CAPS_VNET_HDR |
            QEMU_CAPS_MIGRATE_QEMU_TCP |
            QEMU_CAPS_MIGRATE_QEMU_EXEC |
            QEMU_CAPS_DRIVE_CACHE_V2 |
            QEMU_CAPS_KVM |
            QEMU_CAPS_DRIVE_FORMAT |
            QEMU_CAPS_DRIVE_SERIAL |
            QEMU_CAPS_XEN_DOMID |
            QEMU_CAPS_DRIVE_READONLY |
            QEMU_CAPS_VGA |
            QEMU_CAPS_0_10 |
            QEMU_CAPS_PCIDEVICE |
            QEMU_CAPS_MEM_PATH |
            QEMU_CAPS_SDL |
            QEMU_CAPS_MIGRATE_QEMU_UNIX |
            QEMU_CAPS_CHARDEV |
            QEMU_CAPS_ENABLE_KVM |
            QEMU_CAPS_MONITOR_JSON |
            QEMU_CAPS_BALLOON |
            QEMU_CAPS_DEVICE |
            QEMU_CAPS_SMP_TOPOLOGY |
            QEMU_CAPS_NETDEV |
            QEMU_CAPS_RTC |
            QEMU_CAPS_VNET_HOST |
            QEMU_CAPS_NO_HPET |
            QEMU_CAPS_NO_KVM_PIT |
            QEMU_CAPS_TDF |
            QEMU_CAPS_PCI_CONFIGFD |
            QEMU_CAPS_NODEFCONFIG |
            QEMU_CAPS_BOOT_MENU |
            QEMU_CAPS_FSDEV |
            QEMU_CAPS_NESTING |
            QEMU_CAPS_NAME_PROCESS |
            QEMU_CAPS_SMBIOS_TYPE |
            QEMU_CAPS_SPICE |
            QEMU_CAPS_VGA_NONE |
            QEMU_CAPS_MIGRATE_QEMU_FD |
            QEMU_CAPS_DRIVE_AIO |
            QEMU_CAPS_DEVICE_SPICEVMC,
            13000, 1,  0);
    DO_TEST("qemu-kvm-0.12.1.2-rhel61",
            QEMU_CAPS_VNC_COLON |
            QEMU_CAPS_NO_REBOOT |
            QEMU_CAPS_DRIVE |
            QEMU_CAPS_NAME |
            QEMU_CAPS_UUID |
            QEMU_CAPS_VNET_HDR |
            QEMU_CAPS_MIGRATE_QEMU_TCP |
            QEMU_CAPS_MIGRATE_QEMU_EXEC |
            QEMU_CAPS_DRIVE_CACHE_V2 |
            QEMU_CAPS_KVM |
            QEMU_CAPS_DRIVE_FORMAT |
            QEMU_CAPS_DRIVE_SERIAL |
            QEMU_CAPS_DRIVE_READONLY |
            QEMU_CAPS_VGA |
            QEMU_CAPS_0_10 |
            QEMU_CAPS_PCIDEVICE |
            QEMU_CAPS_MEM_PATH |
            QEMU_CAPS_MIGRATE_QEMU_UNIX |
            QEMU_CAPS_CHARDEV |
            QEMU_CAPS_ENABLE_KVM |
            QEMU_CAPS_BALLOON |
            QEMU_CAPS_DEVICE |
            QEMU_CAPS_SMP_TOPOLOGY |
            QEMU_CAPS_RTC |
            QEMU_CAPS_VNET_HOST |
            QEMU_CAPS_NO_KVM_PIT |
            QEMU_CAPS_TDF |
            QEMU_CAPS_PCI_CONFIGFD |
            QEMU_CAPS_NODEFCONFIG |
            QEMU_CAPS_BOOT_MENU |
            QEMU_CAPS_NESTING |
            QEMU_CAPS_NAME_PROCESS |
            QEMU_CAPS_SMBIOS_TYPE |
            QEMU_CAPS_VGA_QXL |
            QEMU_CAPS_SPICE |
            QEMU_CAPS_VGA_NONE |
            QEMU_CAPS_MIGRATE_QEMU_FD |
            QEMU_CAPS_HDA_DUPLEX |
            QEMU_CAPS_DRIVE_AIO |
            QEMU_CAPS_CCID_PASSTHRU |
            QEMU_CAPS_CHARDEV_SPICEVMC |
            QEMU_CAPS_VIRTIO_TX_ALG,
            12001, 1,  0);

    return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

VIRT_TEST_MAIN(mymain)

#else

int main (void) { return (77); /* means 'test skipped' for automake */ }

#endif /* WITH_QEMU */
