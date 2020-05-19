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

#endif /* Tools_hpp */
