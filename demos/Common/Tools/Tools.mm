//
//  Tools.mm
//  
//
//  Created by Jozef on 18/05/2020.
//

#include <Foundation/Foundation.h>
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
