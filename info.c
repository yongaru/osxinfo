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
#include <time.h>
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
static void print_apple(void);
static void curtime(void);

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
    printf(C4" :ssssssssssssssssssssssss-   ");gpu();
    printf(C5"  osssssssssssssssssssssssso/ ");disk();
    printf(C5"  `syyyyyyyyyyyyyyyyyyyyyyyy+ ");pkg();
    printf(C5"   `ossssssssssssssssssssss/  ");uptime(&now);
    printf(C6"     :ooooooooooooooooooo+.   ");curtime();
    printf(C6"      `:+oo+/:-..-:/+o+/-     \n"C0);
}

static void print_hexley(void) { // thanks cypx for ascii art
    time_t now;
    time(&now);                                          
    printf(C2"                   N                                                     \n");      
    printf(C2"                  $7                                                     \n");       
    printf(C2"              ?   N7N                          Z                         \n");       
    printf(C2"             NN   $77N   7Z~                 ?777                        \n");       
    printf(C2"           =778  D777$   777N          INNNNNN$77N                       \n");        
    printf(C2"          N777$  N777ON  $77Z8      7Z7777777777Z87777777$N              \n");       
    printf(C2"           N7N8=   Z7    7 87N      $777DZ$OD$77777N77777777D            \n");        
    printf(C2"           7Z      878      77N     7777777$MMMND$777Z77777777N          \n");        
    printf(C2"          D7N      D7N      N77=    Z777M88OOOOOOOOD77N77777777$         \n");       
    printf(C2"          N7N      N7N      ?77N    ~77MOO8OOOOOOOOOO$7Z77777777Z        \n");       
    printf(C2"          87N      N7Z       $7O     7NM~OOOOOOOOOOMOO77N77777777N       \n");       
    printf(C2"          87N      N77      Z77D      M  OOOOOOMMDOOMOD77Z7777$NO8NNZ7D  \n");       
    printf(C2"          N77N     N77=     $77D        OOOOOM    :O8OO777777D77777777N  \n");       
    printf(C2"           777$N   N77N  DD777N     MM ~8OOO,      DOON777777O7777777$   \n");       
    printf(C2"            O777777777777777O$   NDOOOMMOOOM       DOO8777777N777777D    \n");      
    printf(C2"              N$7777777778N  :88 OOOOOOOOOO$    MMMOOO77777777DD77Z8     \n");       
    printf(C2"                    $7D       888MND888888OD    MMMOOD77777777777N       \n");       
    printf(C2"                    O7Z        ZM8888888888O    MNOOOD77777777777N       \n");          
    printf(C2"                    N77          O888888M888DOOOOOOOO77777777777N        ");curtime();           
    printf(C2"                   N77+           888888888888888MON777777777NI          ");uptime(&now);          
    printf(C2"                    N77D            ~D8888888888MM8ON777777ND            \n");       
    printf(C2"                    877N             8$8MNDND+$8888O8777$N               \n");       
    printf(C2"                    ?77N            Z8$$$$N7788MOOODZ777$      ████████▄     ▄████████    ▄████████  ▄█     █▄   ▄█  ███▄▄▄▄        ███    █▄  ███▄▄▄▄    ▄█  ▀████    ▐████▀ \n");
    printf(C2"                   :O77NN           M8$$$$$8MOOD    $7777      ███   ▀███   ███    ███   ███    ███ ███     ███ ███  ███▀▀▀██▄      ███    ███ ███▀▀▀██▄ ███    ███▌   ████▀  \n");               
    printf(C2"                   MOOMOOOO         NO8888M~OO8$    Z7777N     ███    ███   ███    ███   ███    ███ ███     ███ ███▌ ███   ███      ███    ███ ███   ███ ███▌    ███  ▐███    \n");             
    printf(C2"                   8OOMZOOOD     MOOO~~~~~~OOOOOO+  D77777O    ███    ███   ███    ███  ▄███▄▄▄▄██▀ ███     ███ ███▌ ███   ███      ███    ███ ███   ███ ███▌    ▀███▄███▀    \n");             
    printf(C2"                    N877OOOOOMMOOOOO~~~~~~~OOOOOOOO.N777777:   ███    ███ ▀███████████ ▀▀███▀▀▀▀▀   ███     ███ ███▌ ███   ███      ███    ███ ███   ███ ███▌    ████▀██      \n");           
    printf(C2"                    8OO78OOOOOOOOOO~~~~~~~~OOOOOOOOO?DN8N$     ███    ███   ███    ███ ▀███████████ ███     ███ ███  ███   ███      ███    ███ ███   ███ ███    ▐███  ▀███    \n");             
    printf(C2"                     N77OOOOOOOOOO$~~~~~~~~OOOOOOOOOOM         ███   ▄███   ███    ███   ███    ███ ███ ▄█▄ ███ ███  ███   ███      ███    ███ ███   ███ ███   ▄███     ███▄  \n");               
    printf(C2"                     877N$OOOOOOON~~~~~~~~~OOO8OOOOOOOM        ████████▀    ███    █▀    ███    ███  ▀███▀███▀  █▀    ▀█   █▀       ████████▀   ▀█   █▀  █▀   ████       ███▄ \n");                
    printf(C2"                     +77N DOOOOOO~~~~~~~~~~~OOONOOOOOOO8                \n");                                                                                                  
    printf(C2"                      77D  .D8M8$~~~~~~~~~~~OOOOMOMOOOOM                \n");          
    printf(C2"                      77Z       ~~~~~~~~~~~~ZOOOOOOOOOO                 ");sysctls(0);          
    printf(C2"                      $77      .~~~~~~~~~~~~~OOOOONOOM                  ");sysctls(3);          
    printf(C2"                      Z77       ~~~~~~~~~~~~~OOOOOO$                    ");sysctls(4);          
    printf(C2"                      877:      ~~~~~~~~~~~~~?OOOOOO                    ");sysctls(5);          
    printf(C2"                      N77Z      M~~~~~~~~~~~~~OOOOOOO                   ");mem();          
    printf(C2"                      N77N     DO=~~~~~~~~~~~IOOOOOOO                   ");sysctls(1);          
    printf(C2"                      N77N    DOOO=~~~~~~~~~DOOOOOOOO                   ");gpu();          
    printf(C2"                      Z77D   MOOOOOD~~~~~~=OOOOOOOOON      MDOODMI      ");disk();         
    printf(C2"                      :77Z   MOOOOOOOD~~~MOOOOOOOOODO MOOOOOOOOOOOO     \n");          
    printf(C2"                       $77.  DOM8OOOOOOOMOOOOOOOOO8OOMOOOOOOOOO8OO7     \n");          
    printf(C2"                       Z778888MMOOOOOOOO8OOOOOOOOONOOOOOOOOOOOON        \n");          
    printf(C2"                       D77N8M8888OOOMD    ~MOOO8OOO     :               \n");          
    printf(C2"                       N7$OI88NN        M88NMDOODOO                     \n");          
    printf(C2"                                       88M888D888D                      \n");          
    printf(C2"                                         NMO M888                       \n");          
}

static void curtime(void) {
    time_t t;
    t = time(NULL);
    printf(RED"Time:     : "NOR"%s", ctime(&t));
}

static void mem(void) {
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
    if (host_page_size(mach_host_self(), &pageSize) == KERN_SUCCESS) {
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

static void gpu(void) { // Thank you bottomy(ScrimpyCat) for this.
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
            "\t-x shows hexley\n"
            "\t-h shows this help msg\n");
    exit(0);
}

static void sysctls(int i) {
    size_t len;
    if (i==2) {
        mem();
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
    if (i == 1) {
        type = getenv("TERM");
        printf(RED"Terminal  : "NOR"%s\n", type);
    } else if (i ==2) {
        passwd = getpwuid(getuid());
        printf(RED"Shell     : "NOR"%s\n",passwd->pw_shell);
    } else if (i == 3) {
        passwd = getpwuid(getuid());
        printf(RED"User      : "NOR"%s\n",passwd->pw_name);
    }
}

static void pkg(void) { // Thank you dcat for this.
    sqlite3 *db;
    int pkgs = 0;
    sqlite3_stmt *s;
    char *sql = "SELECT COUNT (*) FROM LOCAL_PKG";
    if (sqlite3_open(PKGIN_DB, &db) == SQLITE_OK) {
        if (sqlite3_prepare_v2(db, sql, -1, &s, NULL) == SQLITE_OK) {
            if (sqlite3_step(s) != SQLITE_ERROR) {
                pkgs = sqlite3_column_int(s,0);
            }
        }
    }
    sqlite3_close(db);
    printf(RED"Packages  : "NOR"%d\n", pkgs);
}

static void disk(void) {
    struct statvfs info;
    if (!statvfs("/", &info)) {
        unsigned long left  = (info.f_bavail * info.f_frsize);
        unsigned long total = (info.f_files * info.f_frsize);
        unsigned long used  = total - left;
        float perc  = (float)used / (float)total;
        printf(RED"Disk      : "NOR"%.2f%% of %.2f GB\n",
                perc * 100, (float)total / 1e+09);
    }
}

static void uptime(time_t *nowp) {
    struct timeval boottime;
    time_t uptime;
    int days, hrs, mins, secs;
    int mib[2];
    size_t size;
    char buf[256];
    if (strftime(buf, sizeof(buf), NULL, localtime(nowp)))
        mib[0] = CTL_KERN;
    mib[1] = KERN_BOOTTIME;
    size = sizeof(boottime);
    if (sysctl(mib, 2, &boottime, &size, NULL, 0) != -1 &&
            boottime.tv_sec) {
        uptime = *nowp - boottime.tv_sec;
        if (uptime > 60)
            uptime += 30;
        days = (int)uptime / 86400;
        uptime %= 86400;
        hrs = (int)uptime / 3600;
        uptime %= 3600;
        mins = (int)uptime / 60;
        secs = uptime % 60;
        printf(RED"Uptime    : "NOR);
        if (days > 0)
            printf("%d day%s", days, days > 1 ? "s " : " ");
        if (hrs > 0 && mins > 0)
            printf("%02d:%02d", hrs, mins);
        else if (hrs == 0 && mins > 0)
            printf("0:%02d", mins);
        else
            printf("0:00");
        putchar('\n');
    }
}

int main(int argc, char **argv) {
    bool print_with_apple = false;
    bool print_with_hexley = false;
    if (argc >= 2) {
        int c;
        while ((c = getopt(argc, argv, "hax")) != -1) {
            switch (c) {
                case 'a':
                    print_with_apple = true;
                    break;
                case 'x' :
                    print_with_hexley = true;
                    break;
                case 'h':
                default:
                    help();
                    break;
            }
        }
    }
    if (print_with_apple) {
        print_apple();
    } else if (print_with_hexley) {
        print_hexley();
    } else {
        time_t now;
        time(&now);
        envs(3);
        curtime();
        sysctls(0);
        sysctls(3);
        sysctls(4);
        sysctls(5);
        disk();
        mem();
        envs(2);
        envs(1);
        sysctls(1);
        gpu();
        pkg();
        uptime(&now);
    }
    return 0;
}
