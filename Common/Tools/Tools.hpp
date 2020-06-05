//
//  Tools.hpp
//  
//
//  Created by Jozef on 18/05/2020.
//

#ifndef Tools_hpp
#define Tools_hpp

#include <Foundation/Foundation.h>
#include <string_view>

// MARK: - Custom Casts
@interface NSString (alternativeConstructorsCpp)
+ (NSString*)stringFromCppString:(std::string const &)cppString;
- (std::string)cppString;
@end

std::string to_string(const NSString *nsString);

uint64_t mach_time_to_msecs(uint64_t mach_time);
uint64_t msecs_to_mach_time(uint64_t ms);
char *esfflagstostr(uint32_t flags);
char *csflagstostr(uint32_t flags);
std::string faflagstostr(uint32_t flags);


// TODO: demagler
// https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html
template <typename T>
constexpr std::string_view type_name()
{
    std::string_view name, prefix, suffix;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "std::string_view type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr std::string_view type_name() [with T = ";
    suffix = "; std::string_view = std::basic_string_view<char>]";
#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "class std::basic_string_view<char,struct std::char_traits<char> > __cdecl type_name<";
    suffix = ">(void)";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

#endif /* Tools_hpp */
