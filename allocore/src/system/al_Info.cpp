#include <cstdlib> // getenv
#include "allocore/system/al_Config.h"
#include "allocore/system/al_Info.hpp"

#ifdef AL_WINDOWS
#include <windows.h>
#elif AL_OSX
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif

namespace al{

std::string getEnvString(const char * var){
	char * str = std::getenv(var);
	return str ? std::string(str) : "";
};

std::string computerName(){
	#ifdef AL_WINDOWS
	return getEnvString("COMPUTERNAME");
	#else
	return getEnvString("HOSTNAME");
	#endif
}

std::string userName(){
	#ifdef AL_WINDOWS
	return getEnvString("USERNAME");
	#else
	return getEnvString("USER");
	#endif
}

int numProcessors(){
#ifdef AL_WINDOWS
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif AL_OSX
    int nm[2];
    size_t len = 4;
    uint32_t count;

    nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);

    if(count < 1) {
        nm[1] = HW_NCPU;
        sysctl(nm, 2, &count, &len, NULL, 0);
        if(count < 1) { count = 1; }
    }
    return count;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

#ifndef AL_OSX
std::string frameworkResourcePath() {
	return "/";
}
#endif

/// GetX86CpuIDAndInfo - Execute the specified cpuid and return the 4 values in the
/// specified arguments.  If we can't run cpuid on the host, return false.
static bool GetX86CpuIDAndInfo(unsigned value, unsigned *rEAX,
                               unsigned *rEBX, unsigned *rECX, unsigned *rEDX) {
#if defined(__x86_64__) || defined(_M_AMD64) || defined (_M_X64)
#if defined(__GNUC__)
    // gcc doesn't know cpuid would clobber ebx/rbx. Preseve it manually.
    asm ("movq\t%%rbx, %%rsi\n\t"
         "cpuid\n\t"
         "xchgq\t%%rbx, %%rsi\n\t"
         : "=a" (*rEAX),
         "=S" (*rEBX),
         "=c" (*rECX),
         "=d" (*rEDX)
         :  "a" (value));
    return true;
#elif defined(_MSC_VER)
    int registers[4];
    __cpuid(registers, value);
    *rEAX = registers[0];
    *rEBX = registers[1];
    *rECX = registers[2];
    *rEDX = registers[3];
    return true;
#endif
#elif defined(i386) || defined(__i386__) || defined(__x86__) || defined(_M_IX86)
#if defined(__GNUC__)
    asm ("movl\t%%ebx, %%esi\n\t"
         "cpuid\n\t"
         "xchgl\t%%ebx, %%esi\n\t"
         : "=a" (*rEAX),
         "=S" (*rEBX),
         "=c" (*rECX),
         "=d" (*rEDX)
         :  "a" (value));
    return true;
#elif defined(_MSC_VER)
    __asm {
        mov   eax,value
        cpuid
        mov   esi,rEAX
        mov   dword ptr [esi],eax
        mov   esi,rEBX
        mov   dword ptr [esi],ebx
        mov   esi,rECX
        mov   dword ptr [esi],ecx
        mov   esi,rEDX
        mov   dword ptr [esi],edx
    }
    return true;
#endif
#endif
    return false;
}

static void DetectX86FamilyModel(unsigned EAX, unsigned *o_Family, unsigned *o_Model) {
    unsigned Family = (EAX >> 8) & 0xf; // Bits 8 - 11
    unsigned Model  = (EAX >> 4) & 0xf; // Bits 4 - 7
    if (Family == 6 || Family == 0xf) {
        if (Family == 0xf)
            // Examine extended family ID if family ID is F.
            Family += (EAX >> 20) & 0xff;    // Bits 20 - 27
        // Examine extended model ID if family ID is 6 or F.
        Model += ((EAX >> 16) & 0xf) << 4; // Bits 16 - 19
    }

    *o_Family = Family;
    *o_Model = Model;
}

bool is_sandy_bridge() {
    unsigned EAX = 0, EBX = 0, ECX = 0, EDX = 0;
    if(!GetX86CpuIDAndInfo(0x1, &EAX, &EBX, &ECX, &EDX)) return false;

    unsigned Family = 0;
    unsigned Model  = 0;
    DetectX86FamilyModel(EAX, &Family, &Model);

    return Family == 6 && Model == 42;
}

} // al::
