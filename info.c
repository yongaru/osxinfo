/*
 *
 *  Created by yrmt
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <pwd.h>
#include <sys/sysctl.h>
#include <sys/statvfs.h>
#include <sqlite3.h>
#include <mach/mach.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

#define C0  "\x1B[0m"    //Reset
#define C1  "\x1B[0;32m" //Green
#define C2  "\x1B[0;33m" //Yellow
#define C3  "\x1B[1;31m" //RED
#define C4  "\x1B[0;31m" //Red
#define C5  "\x1B[0;35m" //Purple
#define C6  "\x1B[0;36m" //Blue
#define RED C3
#define NOR C0

static const struct {
    const char *ctls;
    const char *names;
} values[] = {
    { "hw.model", "Model" },
    { "machdep.cpu.brand_string", "Processor" },
    { "hw.memsize", "Memory" },
    { "kern.ostype", "OS" },
    { "kern.osrelease", "Kernel" },
    { "kern.hostname", "Hostname" },
};

static void sysctls(int i);
static void envs(int i);
static void disk(void);
static void pkg(void);
static void uptime(time_t *nowp);
static void gpu(void);
static void mem(void);

static void print_apple(void) {
    time_t now;
    time(&now);
    printf(C1"                :++++.        ");envs(3);
    printf(C1"               /+++/.         ");sysctls(0);
    printf(C1"       .:-::- .+/:-``.::-     ");sysctls(3);
    printf(C2"    .:/++++++/::::/++++++/:`  ");sysctls(4);
    printf(C2"  .:///////////////////////:` ");sysctls(5);
    printf(C2"  ////////////////////////`   ");mem();
    printf(C3" -+++++++++++++++++++++++`    ");envs(2);
    printf(C3" /++++++++++++++++++++++/     ");envs(1);
    printf(C4" /sssssssssssssssssssssss.    ");sysctls(1);
    printf(C4" :ssssssssssssssssssssssss-   ");disk();
    printf(C5"  osssssssssssssssssssssssso/ ");pkg();
    printf(C5"  `syyyyyyyyyyyyyyyyyyyyyyyy+ ");uptime(&now);
    printf(C5"   `ossssssssssssssssssssss/  ");gpu();
    printf(C6"     :ooooooooooooooooooo+.   \n");
    printf(C6"      `:+oo+/:-..-:/+o+/-     \n");
}
static void mem(void)
{
    size_t len;
    mach_port_t myHost;
    vm_statistics64_data_t	vm_stat;
    vm_size_t pageSize = 4096; 	/* set to 4k default */
    unsigned int count = HOST_VM_INFO64_COUNT;
    kern_return_t ret;
    myHost = mach_host_self();
    uint64_t value64;
    len = sizeof(value64);
    sysctlbyname(values[2].ctls, &value64, &len, NULL, 0);
    if(host_page_size(mach_host_self(), &pageSize) == KERN_SUCCESS) {
        if ((ret = host_statistics64(myHost, HOST_VM_INFO64, (host_info64_t)&
                                     vm_stat, &count) == KERN_SUCCESS)) {
            printf(RED"%s    : "NOR"%llu MB of %.f MB\n",
                   values[2].names,
                   (uint64_t)(vm_stat.active_count +
                   vm_stat.inactive_count +
                   vm_stat.wire_count)*pageSize >> 20,
                   value64 / 1e+06);
        }
    }
}
static void gpu(void) // Thank you bottomy(ScrimpyCat) for this.
{
    io_iterator_t Iterator;
    kern_return_t err = IOServiceGetMatchingServices(kIOMasterPortDefault,
            IOServiceMatching("IOPCIDevice"),
            &Iterator);
    if(err != KERN_SUCCESS) {
        fprintf(stderr, "IOServiceGetMatchingServices failed: %u\n", err);
    }
    for(io_service_t Device; IOIteratorIsValid(Iterator) &&
            (Device = IOIteratorNext(Iterator)); IOObjectRelease(Device)) {
        CFStringRef Name = IORegistryEntrySearchCFProperty(Device,
                kIOServicePlane,
                CFSTR("IOName"),
                kCFAllocatorDefault,
                kNilOptions);
        if(Name) {
            if(CFStringCompare(Name,CFSTR("display"), 0) == kCFCompareEqualTo) {
                CFDataRef Model = IORegistryEntrySearchCFProperty(Device,
                        kIOServicePlane,
                        CFSTR("model"),
                        kCFAllocatorDefault,
                        kNilOptions);
                if(Model) {
                    bool ValueInBytes = true;
                    CFTypeRef VRAM = IORegistryEntrySearchCFProperty(Device,
                            kIOServicePlane,
                            CFSTR("VRAM,totalsize"),
                            kCFAllocatorDefault,
                            kIORegistryIterateRecursively);
                    if(!VRAM) {
                        ValueInBytes = false;
                        VRAM = IORegistryEntrySearchCFProperty(Device,
                                kIOServicePlane,
                                CFSTR("VRAM,totalMB"),
                                kCFAllocatorDefault,
                                kIORegistryIterateRecursively);
                    }
                    if(VRAM) {
                        mach_vm_size_t Size = 0;
                        CFTypeID Type = CFGetTypeID(VRAM);
                        if(Type==CFDataGetTypeID())
                            Size=(CFDataGetLength(VRAM) == sizeof(uint32_t) ?
                    (mach_vm_size_t)*(const uint32_t*)CFDataGetBytePtr(VRAM):
                                    *(const uint64_t*)CFDataGetBytePtr(VRAM));
                        else if(Type == CFNumberGetTypeID())
                            CFNumberGetValue(VRAM,
                                    kCFNumberSInt64Type, &Size);
                        if(ValueInBytes) Size >>= 20;
                        printf(RED"Graphics  : "NOR"%s @ %llu MB\n",
                                CFDataGetBytePtr(Model),Size);
                        CFRelease(Model);
                    }
                    else printf(RED"Graphic  : "NOR"%s @ Unknown VRAM Size\n",
                            CFDataGetBytePtr(Model));
                    CFRelease(Model);
                }
            }
            CFRelease(Name);
        }
    }
}
void help(void) {
    printf("Mac OS X Info program by yrmt dec. 2013\n"
            "\t-a shows a colored apple\n"
            "\t-h shows this help msg\n");
    exit(0);
}
static void sysctls(int i) {
    size_t len;
    uint64_t value64;
    if(i == 2) {
        len = sizeof(value64);
        sysctlbyname(values[2].ctls, &value64, &len, NULL, 0);
        printf(RED"%-10s: "NOR"%.f MB\n", values[2].names, value64/1e+06);
    } else {
        sysctlbyname(values[i].ctls, NULL, &len, NULL, 0);
        char *type = malloc(len);
        sysctlbyname(values[i].ctls, type, &len, NULL, 0);
        printf(RED"%-10s: "NOR"%s\n", values[i].names, type);
        free(type);
    }
}
static void envs(int i) {
    char *type;
    struct passwd *passwd;
    if(i == 1) {
        type = getenv("TERM");
        printf(RED"Terminal  : "NOR"%s\n", type);
    } else if(i ==2) {
        passwd = getpwuid(getuid());
        printf(RED"Shell     : "NOR"%s\n",passwd->pw_shell);
    } else if(i == 3) {
        passwd = getpwuid(getuid());
        printf(RED"User      : "NOR"%s\n",passwd->pw_name);
    }
}
static void pkg(void) { // Thank you dcat for this.
    sqlite3 *db;
    int pkgs = 0;
    sqlite3_stmt *s;
    char *sql = "SELECT COUNT (*) FROM LOCAL_PKG";
    if(sqlite3_open("/var/db/pkgin/pkgin.db", &db) == SQLITE_OK) {
        if(sqlite3_prepare_v2(db, sql, -1, &s, NULL) == SQLITE_OK) {
            if(sqlite3_step(s) != SQLITE_ERROR) {
                pkgs = sqlite3_column_int(s,0);
            }
        }
    }
    sqlite3_close(db);
    printf(RED"Packages  : "NOR"%d\n", pkgs);
}
static void disk(void) {
    struct statvfs info;
    if(!statvfs("/", &info)) {
        unsigned long left  = (info.f_bavail * info.f_frsize);
        unsigned long total = (info.f_files * info.f_frsize);
        unsigned long used  = total - left;
        float perc  = (float)used / (float)total;
        printf(RED"Disk      : "NOR"%.2f%% of %.2f GB\n",
                perc * 100, (float)total / 1e+09);
    }
}
static void uptime(time_t *nowp)
{
    struct timeval boottime;
    time_t uptime;
    int days, hrs, mins, secs;
    int mib[2];
    size_t size;
    char buf[256];
    if(strftime(buf, sizeof(buf), NULL, localtime(nowp)))
        mib[0] = CTL_KERN;
    mib[1] = KERN_BOOTTIME;
    size = sizeof(boottime);
    if(sysctl(mib, 2, &boottime, &size, NULL, 0) != -1 &&
            boottime.tv_sec) {
        uptime = *nowp - boottime.tv_sec;
        if(uptime > 60)
            uptime += 30;
        days = (int)uptime / 86400;
        uptime %= 86400;
        hrs = (int)uptime / 3600;
        uptime %= 3600;
        mins = (int)uptime / 60;
        secs = uptime % 60;
        printf(RED"Uptime    : "NOR);
        if(days > 0)
            printf("%d day%s", days, days > 1 ? "s " : " ");
        if (hrs > 0 && mins > 0)
            printf("%2d:%02d", hrs, mins);
        else if(hrs > 0)
            printf("%d hr%s", hrs, hrs > 1 ? "s " : " ");
        else if(mins > 0)
            printf("%d min%s", mins, mins > 1 ? "s " : " ");
        else
            printf("%d sec%s", secs, secs > 1 ? "s " : " ");
        putchar('\n');
    }
}
int main(int argc, char **argv) {
    bool print_with_apple = false;
    if(argc >= 2) {
        int c;
        while((c = getopt(argc, argv, "ha")) != -1) {
            switch(c) {
                case 'a':
                    print_with_apple = true;
                    break;
                case 'h':
                default:
                    help();
                    break;
            }
        }
    }
    if(print_with_apple) {
        print_apple();
    } else {
        time_t now;
        time(&now);
        envs(3);
        sysctls(0);
        sysctls(3);
        sysctls(4);
        sysctls(5);
        disk();
        mem();
        envs(2);
        envs(1);
        pkg();
        uptime(&now);
    }
    return 0;
}