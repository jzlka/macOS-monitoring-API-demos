//
//  Tools.mm
//  
//
//  Created by Jozef on 18/05/2020.
//

#include <Foundation/Foundation.h>
#include <Kernel/kern/cs_blobs.h>
#include <mach/mach_time.h>
#include <stdexcept>
#include <string>
#include <sys/fcntl.h>
#include "Tools.hpp"

// MARK: - Custom Casts
@implementation NSString (alternativeConstructorsCpp)

+(NSString*)stringFromCppString:(std::string const &)cppString
{
    return [[NSString alloc] initWithCString:cppString.c_str() encoding:NSStringEncodingConversionAllowLossy];
}

-(std::string)cppString
{
    return std::string([self cStringUsingEncoding:NSStringEncodingConversionAllowLossy]);
}

@end

std::string to_string(const NSString *nsString)
{
    return std::string([nsString cStringUsingEncoding:NSStringEncodingConversionAllowLossy]);
}


// https://gist.github.com/leiless/de908154e4c1952186069fe330680b70
uint64_t mach_time_to_msecs(uint64_t mach_time)
{
    mach_timebase_info_data_t tb;
    kern_return_t e = mach_timebase_info(&tb);
    if (e != KERN_SUCCESS)
        throw std::invalid_argument("Could not convert mach time to msecs!");
    return (mach_time * tb.numer) / (tb.denom * NSEC_PER_MSEC);
}

uint64_t msecs_to_mach_time(uint64_t ms)
{
    mach_timebase_info_data_t tb;
    kern_return_t e = mach_timebase_info(&tb);
    if (e != KERN_SUCCESS)
        throw std::invalid_argument("Could not convert mach time to msecs!");
    return (ms * tb.denom * NSEC_PER_MSEC) / tb.numer;
}


#define longestesfflaglen    11
static struct {
    char name[longestesfflaglen + 1];
    uint32_t flag;
} const esmapping[] = {
    { "FREAD",          FREAD },
    { "FWRITE",         FWRITE },
    { "FAPPEND",        FAPPEND },
    { "FASYNC",         FASYNC },
    { "FFSYNC",         FFSYNC },
    { "FFDSYNC",        FFDSYNC },
    { "FNONBLOCK",      FNONBLOCK },
    { "FNDELAY",        FNDELAY },
    { "O_NDELAY",       O_NDELAY },
    { "O_SHLOCK",       O_SHLOCK },
    { "O_EXLOCK",       O_EXLOCK },
    { "O_NOFOLLOW",     O_NOFOLLOW },
    { "O_CREAT",        O_CREAT },
    { "O_TRUNC",        O_TRUNC },
    { "O_EXCL",         O_EXCL },
    { "O_DIRECTORY",    O_DIRECTORY },
    { "O_SYMLINK",      O_SYMLINK },
};
#define nesmappings    (sizeof(esmapping) / sizeof(esmapping[0]))

// Based on freebsd/lib/libc/gen/strtofflags.c
char *esfflagstostr(uint32_t flags)
{
    char *string;
    const char *sp;
    char *dp;
    uint32_t setflags;
    u_int i;

    if ((string = (char *)malloc(nesmappings * (longestesfflaglen + 1))) == NULL)
        return (NULL);

    setflags = flags;
    dp = string;
    for (i = 0; i < nesmappings; i++) {
        if (setflags & esmapping[i].flag) {
            if (dp > string)
                *dp++ = ',';
            for (sp = esmapping[i].name; *sp; *dp++ = *sp++) ;
            setflags &= ~esmapping[i].flag;
        }
    }
    *dp = '\0';
    return (string);
}


#define longestcsflaglen    25
static struct {
    char name[longestcsflaglen + 1];
    uint32_t flag;
} const csmapping[] = {
    { "CS_VALID",                   CS_VALID },
    { "CS_ADHOC",                   CS_ADHOC },
    { "CS_GET_TASK_ALLOW",          CS_GET_TASK_ALLOW },
    { "CS_INSTALLER",               CS_INSTALLER },
    { "CS_FORCED_LV",               CS_FORCED_LV },
    { "CS_INVALID_ALLOWED",         CS_INVALID_ALLOWED },
    { "CS_HARD",                    CS_HARD },
    { "CS_KILL",                    CS_KILL },
    { "CS_CHECK_EXPIRATION",        CS_CHECK_EXPIRATION },
    { "CS_RESTRICT",                CS_RESTRICT },
    { "CS_ENFORCEMENT",             CS_ENFORCEMENT },
    { "CS_REQUIRE_LV",              CS_REQUIRE_LV },
    { "CS_ENTITLEMENTS_VALIDATED",  CS_ENTITLEMENTS_VALIDATED },
    { "CS_NVRAM_UNRESTRICTED",      CS_NVRAM_UNRESTRICTED },
    { "CS_RUNTIME",                 CS_RUNTIME },
    { "CS_EXEC_SET_HARD",           CS_EXEC_SET_HARD },
    { "CS_EXEC_SET_KILL",           CS_EXEC_SET_KILL },
    { "CS_EXEC_SET_ENFORCEMENT",    CS_EXEC_SET_ENFORCEMENT },
    { "CS_EXEC_INHERIT_SIP",        CS_EXEC_INHERIT_SIP },
    { "CS_KILLED",                  CS_KILLED },
    { "CS_DYLD_PLATFORM",           CS_DYLD_PLATFORM },
    { "CS_PLATFORM_BINARY",         CS_PLATFORM_BINARY },
    { "CS_PLATFORM_PATH",           CS_PLATFORM_PATH },
    { "CS_DEBUGGED",                CS_DEBUGGED },
    { "CS_SIGNED",                  CS_SIGNED },
    { "CS_DEV_CODE",                CS_DEV_CODE },
    { "CS_DATAVAULT_CONTROLLER",    CS_DATAVAULT_CONTROLLER },
};
#define ncsmappings    (sizeof(csmapping) / sizeof(csmapping[0]))

char *csflagstostr(uint32_t flags)
{
    char *string;
    const char *sp;
    char *dp;
    uint32_t setflags;
    u_int i;

    if ((string = (char *)malloc(ncsmappings * (longestcsflaglen + 1))) == NULL)
        return (NULL);

    setflags = flags;
    dp = string;
    for (i = 0; i < ncsmappings; i++) {
        if (setflags & csmapping[i].flag) {
            if (dp > string)
                *dp++ = ',';
            for (sp = csmapping[i].name; *sp; *dp++ = *sp++) ;
            setflags &= ~csmapping[i].flag;
        }
    }
    *dp = '\0';
    return (string);
}
