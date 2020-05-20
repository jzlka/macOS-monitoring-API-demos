//
//  Tools.mm
//  
//
//  Created by Jozef on 18/05/2020.
//

#include <Foundation/Foundation.h>
#include <mach/mach_time.h>
#include <stdexcept>
#include <string>
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
