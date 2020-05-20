//
//  Tools.hpp
//  
//
//  Created by Jozef on 18/05/2020.
//

#ifndef Tools_hpp
#define Tools_hpp

// MARK: - Custom Casts
@interface NSString (alternativeConstructorsCpp)
+ (NSString*)stringFromCppString:(std::string const &)cppString;
- (std::string)cppString;
@end

uint64_t mach_time_to_msecs(uint64_t mach_time);
uint64_t msecs_to_mach_time(uint64_t ms);

#endif /* Tools_hpp */
