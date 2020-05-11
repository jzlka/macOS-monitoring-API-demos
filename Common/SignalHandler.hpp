//
//  SignalHandler.hpp
//  
//
//  Created by Jozef on 11/05/2020.
//

#ifndef SignalHandler_hpp
#define SignalHandler_hpp

#include <CoreFoundation/CoreFoundation.h>

void HandleSIGTERMFromRunLoop(CFFileDescriptorRef f, CFOptionFlags callBackTypes, void *info);
void InstallHandleSIGTERMFromRunLoop();

#endif /* SignalHandler_hpp */
